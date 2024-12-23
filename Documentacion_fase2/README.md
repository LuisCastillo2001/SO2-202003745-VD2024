# Proyecto 2 Asignador de memoria

Un **asignador de memoria** (o **gestor de memoria**) es un componente del sistema operativo o una librería que se encarga de administrar la memoria dinámica en un programa. Tiene como función principal es proporcionar a las aplicaciones bloques de memoria cuando lo necesitan y liberar esa memoria cuando ya no se usa, asegurando un uso eficiente de los recursos disponibles.

## Resumen del proyecto

El proyecto tiene como objetivo el desarrollo de una nueva variante en la familia de asignadores de memoria, denominada **tamalloc**, que asegura la inicialización de la memoria en 0 sin marcar toda su memoria como utilizada de inmediato. Esta variante inicializa las páginas pertenecientes a la memoria reservada una por una según sean accedidas, lo que permite que la memoria en estado RSS no se dispare de inmediato. Esto contribuye a mantener la memoria sin páginas físicas inmediatamente, permitiendo al sistema un mejor manejo del over-commit.

## Implementación de tamalloc con lazy-zeoring

Antes de explicar la implementación de tamalloc, se dará una breve explicación de como funciona malloc.

- **Malloc**: En programación en C, **`malloc`** (de **memory allocation**, o **asignación de memoria**) es una función que se utiliza para asignar memoria dinámica en el heap durante la ejecución del programa. La memoria asignada con `malloc` puede ser utilizada para almacenar datos mientras dure el programa.

En palabras simples, podemos relacionar malloc con el siguiente ejemplo

Imagina que estás en una tienda de muebles y quieres comprar una mesa para tu oficina. La tienda tiene varios tamaños de mesas disponibles, pero no sabes exactamente cuál necesitas. Entonces, le pides al encargado una mesa del tamaño que consideres adecuado para tu oficina en ese momento.

El encargado de la tienda, al recibir tu solicitud, va al almacén y selecciona una mesa del tamaño que pediste. Te la lleva y te la entrega para que la utilices. Sin embargo, la mesa está vacía: no tiene ningún contenido o muebles en ella, simplemente es una mesa libre de elementos.

### Relación con `malloc`

- **La tienda**: Representa el sistema operativo.
- **El encargado**: Es el proceso que gestiona la memoria (en este caso, el sistema operativo).
- **La mesa**: Representa el bloque de memoria.
- **El tamaño que pides**: Es el número de bytes que deseas asignar.

Una vez claros con los conceptos de que es un asignador de memoria, podemos explicar la implementación y como funciona tamalloc.

### Tamalloc

```c
SYSCALL_DEFINE2(luis_tamalloc, size_t, size, unsigned long __user *, addr)
{
    unsigned long user_addr;


    size = PAGE_ALIGN(size);


    user_addr = vm_mmap(NULL, 0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, 0);
    if (IS_ERR_VALUE(user_addr)) {
        return user_addr;
    }

    // Copiar la dirección de memoria asignada al espacio de usuario
    if (copy_to_user(addr, &user_addr, sizeof(user_addr))) {
        vm_munmap(user_addr, size);
        return -EFAULT;
    }

    return 0;
};



static vm_fault_t tamalloc_page_fault_handler(struct vm_fault *vmf)
{
    struct page *page;
    void *page_addr;


    page = alloc_page(GFP_KERNEL);
    if (!page)
        return VM_FAULT_OOM;


    page_addr = page_address(page);
    memset(page_addr, 0, PAGE_SIZE);


    vmf->page = page;

    return 0;
};

static const struct vm_operations_struct tamalloc_vm_ops = {
    .fault = tamalloc_page_fault_handler,
};
```

En el primer proyecto se hizo una explicación de como se implementan las syscalls, y sus definiciones, así que en esta documentación no se profundizará en este tema.

#### Términos importantes en el código y su respectiva explicación

```c
size = PAGE_ALIGN(size);
```

**`PAGE_ALIGN(size)`**: Alinea el tamaño de la memoria al tamaño de una página de memoria. Las páginas de memoria suelen tener un tamaño fijo, como 4KB o 8KB. Esto asegura que el tamaño solicitado sea un múltiplo de este tamaño de página, lo cual es importante para la administración de memoria en el sistema operativo.

```c
user_addr = vm_mmap(NULL, 0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, 0);
```

**`vm_mmap`**: Es una función del núcleo de Linux que asigna memoria al proceso. En este caso, se está pidiendo un bloque de memoria de tamaño `size` con permisos de lectura y escritura.

- **`NULL`**: El primer parámetro es `NULL` porque no se está pasando una dirección específica; se le deja al sistema decidir la dirección de inicio.
- **`0`**: El segundo parámetro, que es 0, le dice al sistema que no tiene que inicializar un valor de dirección.
- **`PROT_READ | PROT_WRITE`**: Define los permisos de la memoria, en este caso, se le da permiso para leer y escribir.
- **`MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE`**:
  - **`MAP_PRIVATE`**: La memoria asignada no será compartida entre procesos.
  - **`MAP_ANONYMOUS`**: La memoria no se conecta a un archivo, es solo un bloque de memoria anónima.
  - **`MAP_NORESERVE`**: No reserva físicamente la memoria, lo que significa que no se asignan páginas de memoria reales hasta que se accede a ellas.

### Fallos de página

Las estructuras que ves (`vm_fault_t`, `vm_operations_struct`, `tamalloc_page_fault_handler`) no mencionan explícitamente a `tamalloc`, pero están relacionadas con la forma en que funciona la gestión de memoria en el sistema operativo, y especialmente con cómo se maneja la **asignación perezosa de memoria** (o "lazy allocation"), que es parte de la implementación de `tamalloc`.

A continuación se desglosará como se relacionan con tamalloc y que tan importantes son estas funciones.

```c
static vm_fault_t tamalloc_page_fault_handler(struct vm_fault *vmf)
{
    struct page *page;
    void *page_addr;

    // Asignación de una nueva página de memoria
    page = alloc_page(GFP_KERNEL);
    if (!page)
        return VM_FAULT_OOM;  // Error si no hay suficiente memoria

    // Obtener la dirección de la página asignada
    page_addr = page_address(page);
    // Inicializar la página a cero
    memset(page_addr, 0, PAGE_SIZE);

    // Establecer la página en el fallo de página
    vmf->page = page;

    return 0;  // Indica que se manejó el fallo correctamente
};
```

**`tamalloc_page_fault_handler`**: Esta es la función que maneja el fallo de página. Cuando el proceso intenta acceder a una página de memoria que no ha sido inicializada, esta función:

1. Asigna una nueva **página de memoria** (usando **`alloc_page`**).
2. **Inicializa** esta página con ceros (`memset`), como parte del comportamiento de `tamalloc` para asegurar que las páginas sean inicializadas solo cuando se acceden por primera vez.
3. **Asocia** esta página recién asignada al fallo de página (`vmf->page`), lo que le dice al sistema que esta página ahora está disponible para el proceso.

Si no hay suficiente memoria disponible para asignar una nueva página, la función devuelve un error **`VM_FAULT_OOM`** (fuera de memoria).

Estas estructuras y funciones son parte de la **implementación de la asignación perezosa** de `tamalloc`. La idea es que la memoria no se asigna ni se inicializa hasta que el proceso realmente la necesita. Cuando ocurre un fallo de página (es decir, cuando el proceso intenta acceder a una página de memoria no inicializada), el **manejador de fallos de página** (`tamalloc_page_fault_handler`) se activa y asigna la memoria solo en ese momento.

El sistema operativo usa un mecanismo llamado **fallo de página** (page fault) para detectar cuando un programa intenta acceder a una parte de la memoria que aún no se ha asignado físicamente.

### Ejemplo sencillo de cómo funciona el manejador de fallos de página

1. **Petición de memoria**:
   
   - Supongamos que se tiene un programa que pide memoria usando `tamalloc`. Digamos que el programa necesita 4 KB de memoria.
   - El sistema **no asigna inmediatamente esa memoria física** en RAM. Solo marca esa memoria como "reservada" en el espacio virtual del programa, pero no la carga en la memoria RAM.

2. **Acceso a la memoria (Causa de un fallo de página)**:
   
   - Ahora, el programa intenta **acceder a esa memoria**. Puede ser, por ejemplo, intentando escribir un valor en esa dirección de memoria.
   - En este punto, como el sistema no ha asignado aún físicamente esa memoria (es decir, aún no está en la RAM), ocurre un **"fallo de página"**.

3. **Manejo del fallo de página**:
   
   - El sistema detecta que el programa intentó acceder a una parte de memoria que aún no existe en la RAM. Entonces, se activa el **manejador de fallos de página**, que es la función **`tamalloc_page_fault_handler`**.
   - Esta función se encarga de:
     1. **Asignar una nueva página de memoria física**.
     2. **Inicializarla a cero** (en el caso de `tamalloc`, ya que la memoria debe empezar vacía).
     3. **Dar esa página al proceso**, lo que significa que el proceso ahora puede seguir usando esa parte de memoria.

Con esta explicación se dieron los conceptos fundamentales para entender el uso y funcionamiento de tamalloc.

## Funcionamiento de tamalloc

![](assets/2024-12-23-13-17-19-image.png)

En términos generales, lo que hace **tamalloc** es una **asignación de memoria perezosa**, donde la memoria no se asigna físicamente en el momento de la solicitud, sino que se asigna solo cuando se accede a ella. Esto ayuda a optimizar el uso de la memoria y el manejo del over-commit del sistema.

Basado en la salida que compartiste, aquí hay un desglose de lo que sucede a un nivel más general:

1. **Solicitud de memoria:**
   
   - El programa solicita **10 MB** de memoria utilizando `tamalloc`. En este punto, la memoria no se asigna de inmediato. En lugar de eso, **el sistema marca esa memoria como "reservada"** en el espacio de direcciones del proceso. No hay memoria física asignada aún.

2. **Dirección asignada:**
   
   - Una vez que el sistema ha marcado la memoria como reservada, se devuelve la dirección donde **esa memoria virtual está reservada** para el programa (en este caso, la dirección `0x71f449400000`). Es importante notar que esta dirección es virtual, no física. Aún no hay memoria RAM asignada.

3. **Acceso a la memoria (activación de la asignación perezosa):**
   
   - Cuando el programa empieza a **leer la memoria byte por byte**, es en este momento cuando **la asignación perezosa entra en acción**. El primer acceso a una dirección de memoria que estaba reservada pero no asignada físicamente **provoca un fallo de página**, lo que hace que el sistema operativo asigne físicamente esa memoria en RAM.
   - **El fallo de página** activa el manejador que asigna la memoria física solo cuando el programa realmente intenta usarla, lo que ahorra recursos si nunca se accede a esa memoria.

4. **Verificación de la inicialización:**
   
   - Después de que la memoria se asigna, el programa verifica que cada byte de la memoria está **inicializado a cero**. Como el sistema está configurado para inicializar la memoria a cero en el momento del fallo de página, el programa confirma que la memoria está **correctamente inicializada en cero**.
   - Cada vez que el programa verifica 1 MB de memoria, se imprime un mensaje indicando que ese segmento ha sido verificado.

5. **Escritura en la memoria:**
   
   - El programa luego escribe valores aleatorios en la memoria, lo que indica que ahora está usando activamente esa memoria asignada.

6. **Verificación final:**
   
   - Al final, el programa confirma que toda la memoria solicitada ha sido correctamente inicializada a cero y luego escrita.
