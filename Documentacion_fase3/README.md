# Proyecto 3 : Limitador de memoria para procesos en linux

## Descripción del proyecto

Hay procesos que acaparan muchísima memoria si la encuentran disponible en el sistema, dejando a otros procesos sin memoria. Por lo que se debe implementar un sistema de limitación de recursos que les permita poner un límite a la cantidad de memoria máxima que los procesos pueden solicitar dinámicamente. Esto se deberá hacer a través de un control en memoria del kernel, el cual monitoree con ayuda de una estructura de lista dinámica que procesos deben ser limitados y cual es el valor máximo de memoria para cada uno de ellos.

## Implementación del proyecto

Anteriormente en la documentación del proyecto 1 se puede encontrar como implementar syscalls, por lo tanto no se tocará este tema en concreto.

No obstante ahora el proyecto se hizo con una nueva funcionalidad, y esta es realizar las syscalls en un archivo por separado, para que se vea más ordenado, y ya no fuese necesario implementar todo en sys.c.

## Separar carpetas del kernel

Para poder separarlas, en el makefile del kernel se agrego la siguiente línea:

```c
LINUXINCLUDE += -I$(srctree)/kernel/fase3
```

En el directorio /linux6.8/kernel/Makefile

```c
obj-y += entry/
obj-y += fase3/
```

Y en la carpeta fase 3 se realizo el siguiente makefile

```c
obj-y += syscall3.o
```

Con esto ya podemos obtener nuestras syscalls en archivos por separado para tener una mejor estructura de nuestro kernel.

## Explicación de las syscalls implementadas

Para la realización de este proyecto, se hizo uso de una lista enlazada doble, la cuál para recordar, se podría decir que es de la siguiente manera:

![LISTAS DOBLEMENTE-ENLAZADAS](https://ccia.ugr.es/~jfv/ed1/tedi/cdrom/icons/lenlaz3.gif)

El código implementado para la realización de la lista enlazada fue el siguiente:

```c
struct memory_limitation {
    pid_t pid;
    size_t memory_limit;
};

struct memory_limitation_list {
    struct list_head list; // Encabezado de la lista
    struct memory_limitation node; // Nodo con datos
};

// Lista principal
static LIST_HEAD(memory_limitation_head);
```

- **`struct memory_limitation`**:
  
  - Es una estructura que contiene información sobre un proceso específico:
    - `pid`: El ID del proceso al que se aplicará una limitación de memoria.
    - `memory_limit`: La cantidad de memoria máxima permitida para ese proceso.

- **`struct memory_limitation_list`**:
  
  - Es una estructura que combina la información de `memory_limitation` con los elementos necesarios para integrarla en una lista enlazada doble (usando `struct list_head`).

- **`static LIST_HEAD(memory_limitation_head)`**:
  
  - Es una macro del kernel que inicializa una lista enlazada doble llamada `memory_limitation_head`.
  - Actúa como el "nodo cabeza" o punto de entrada de la lista.

Toda esta gestión de procesos tiene una gestión CRUD

- Create process

- Remove process

- Update process

- Delete process

A continuación se mostrará la implementación de cada uno de estos en el código, no obstante se dará una breve explicación de como se realizó el límite usando rlimit, el código usado para cambiar el límite de procesos fue el siguiente: 

```c
static int set_process_memory_limit(struct task_struct *task, size_t memory_limit) {
    struct rlimit new_rlimit;

    new_rlimit.rlim_cur = memory_limit;
    new_rlimit.rlim_max = memory_limit;

    return do_prlimit(task, RLIMIT_AS, &new_rlimit, NULL);
} 
```

### **¿Qué es `struct rlimit`?**

`struct rlimit` es una estructura utilizada para definir límites de recursos para un proceso en Linux.  
Contiene dos campos principales:

- **`rlim_cur`**: Representa el límite "suave" (soft limit). Es el valor actual que se aplica al recurso.
- **`rlim_max`**: Representa el límite "duro" (hard limit). Es el valor máximo que puede establecer un proceso, y solo puede ser modificado por un usuario con privilegios (como root).

En este caso, el recurso que se controla es **`RLIMIT_AS`**, que corresponde al límite de espacio de direcciones virtuales de un proceso (es decir, cuánta memoria puede asignar).

## ¿Qué hace la función `set_process_memory_limit`?

- **Recibe como parámetros:**
  
  - `struct task_struct *task`: Una estructura que representa el proceso al que se aplicará el límite de memoria.
  - `size_t memory_limit`: El límite de memoria que se quiere establecer, en bytes.

- **Crea un nuevo límite (`new_rlimit`):**
  
  - Se establece tanto el límite "suave" (`rlim_cur`) como el "duro" (`rlim_max`) al valor deseado (`memory_limit`).

- **Llama a `do_prlimit`:**
  
  - **`do_prlimit`**: Es una función del kernel que modifica los límites de recursos de un proceso.
  - Parámetros:
    - `task`: El proceso al que se aplicará el cambio.
    - `RLIMIT_AS`: El tipo de recurso que se está limitando (en este caso, el espacio de direcciones virtuales).
    - `&new_rlimit`: El nuevo valor del límite.
    - 

- **Devuelve el resultado de `do_prlimit`:**
  
  - Si se establece correctamente, devuelve 0.
  - Si hay algún error (por ejemplo, si no tienes permisos), devolverá un código de error negativo.

## Funciones adicionales

**`find_node_by_pid`**

```c
static struct memory_limitation_list *find_node_by_pid(pid_t pid) {
    struct memory_limitation_list *entry;

    list_for_each_entry(entry, &memory_limitation_head, list) {
        if (entry->node.pid == pid) {
            return entry; // Nodo encontrado
        }
    }

    return NULL; // Nodo no encontrado
}
```

- Busca en la lista enlazada un nodo que coincida con el `pid` proporcionado.
- Recorre la lista usando `list_for_each_entry`.
- Si encuentra un nodo con el `pid` correspondiente, lo devuelve.
- Si no lo encuentra, retorna `NULL`.

**`get_process_memory_usage`**

```c
static size_t get_process_memory_usage(struct task_struct *task) {
    struct mm_struct *mm;
    size_t memory_usage = 0;

    mm = get_task_mm(task);
    if (mm) {
        memory_usage = mm->total_vm << PAGE_SHIFT;
        mmput(mm);
    }
    return memory_usage;
}
```

- Obtiene la estructura `mm_struct` del proceso (que contiene información de memoria).
- Calcula el uso total de memoria virtual (`total_vm`) del proceso, convirtiéndolo a bytes (`<< PAGE_SHIFT`).
- Libera la referencia a `mm_struct` usando `mmput`.

**`add_memory_limitation_node`**

```c
static int add_memory_limitation_node(pid_t pid, size_t memory_limit) {
    struct memory_limitation_list *new_entry;

    // Verificar si ya existe un nodo con el mismo PID
    if (find_node_by_pid(pid)) {
        return -101; // Nodo ya existe
    }

    // Asignar memoria para la nueva entrada
    new_entry = kmalloc(sizeof(struct memory_limitation_list), GFP_KERNEL);
    if (!new_entry) {
        return -ENOMEM; // Error de memoria
    }

    // Inicializar la entrada
    new_entry->node.pid = pid;
    new_entry->node.memory_limit = memory_limit;
    INIT_LIST_HEAD(&new_entry->list);

    // Agregar a la lista principal
    list_add(&new_entry->list, &memory_limitation_head);

    return 0; // Nodo agregado exitosamente
} 
```

- Crea y agrega un nodo con la información del proceso (`pid` y `memory_limit`) a la lista enlazada.
- Verifica si el `pid` ya está en la lista para evitar duplicados.
- Asigna memoria para el nuevo nodo con `kmalloc` e inicializa sus valores.
- Agrega el nodo a la lista principal (`memory_limitation_head`) usando `list_add`.

## Añadir el límite para un proceso

```c
SYSCALL_DEFINE2(luis_add_memory_limit, pid_t, process_pid, size_t, memory_limit) {
    struct task_struct *task;
    struct memory_limitation_list *existing_node;
    size_t current_memory_usage;
    int ret;

    // Verificar permisos
    if (!capable(CAP_SYS_ADMIN)) {
        return -EPERM;
    }

    // Validar argumentos
    if (process_pid <= 0 || memory_limit <= 0) {
        return -EINVAL; // Argumento inválido
    }

    // Buscar el proceso por PID
    task = find_task_by_vpid(process_pid);
    if (!task) {
        return -ESRCH; // Proceso no encontrado
    }

    // Verificar si ya existe el nodo en la lista
    existing_node = find_node_by_pid(process_pid);
    if (existing_node) {
        return -101; // Proceso ya en la lista
    }

    // Verificar el uso actual de memoria
    current_memory_usage = get_process_memory_usage(task);
    if (current_memory_usage > memory_limit) {
        return -100; // Límite excedido
    }

    // Establecer el límite de memoria
    ret = set_process_memory_limit(task, memory_limit);
    if (ret) {
        return ret; // Error al establecer el límite
    }

    return add_memory_limitation_node(process_pid, memory_limit);
}
```

- **Verificación de permisos**:
  
  - Utiliza la función `capable(CAP_SYS_ADMIN)` para comprobar si el usuario tiene privilegios de administrador (`root`).
  - Si no tiene permisos, la syscall devuelve `-EPERM` (operación no permitida).

- **Validación de argumentos**:
  
  - Revisa que el `pid` (`process_pid`) y el límite de memoria (`memory_limit`) sean valores positivos.
  - Si alguno de los argumentos es inválido (e.g., negativos o cero), devuelve `-EINVAL` (argumento inválido).

- **Búsqueda del proceso**:
  
  - Usa `find_task_by_vpid` para buscar el proceso con el `pid` proporcionado.
  - Si el proceso no existe, devuelve `-ESRCH` (proceso no encontrado).

- **Comprobación de existencia y uso actual de memoria**:
  
  - Llama a `find_node_by_pid` para verificar si el proceso ya tiene un nodo en la lista enlazada de procesos con límite de memoria. Si el nodo ya existe, devuelve `-101`.
  - Calcula el uso actual de memoria del proceso con `get_process_memory_usage`. Si este uso ya excede el nuevo límite, devuelve `-100`.

- **Establecimiento del límite y registro**:
  
  - Llama a `set_process_memory_limit` para establecer el nuevo límite de memoria para el proceso. Si ocurre un error, retorna el código correspondiente.
  - Finalmente, utiliza `add_memory_limitation_node` para registrar el proceso con su límite en la lista enlazada principal, y devuelve `0` si todo fue exitoso.

## Obtener los procesos limitados

```c
SYSCALL_DEFINE3(luis_get_memory_limits, struct memory_limitation*, u_processes_buffer, size_t, max_entries, int*, processes_buffer_size) {
    struct memory_limitation_list *entry; 
    int count = 0;

    // Validar el valor de max_entries
    if (max_entries <= 0) {
        return -EINVAL;
    }

    // Validar los punteros de espacio de usuario
    if (!access_ok(u_processes_buffer, max_entries * sizeof(struct memory_limitation)) ||
        !access_ok(processes_buffer_size, sizeof(int))) {
        return -EINVAL;
    }

    // Reservar memoria en espacio de kernel
    struct memory_limitation* k_processes_buffer = kmalloc_array(max_entries, sizeof(struct memory_limitation), GFP_KERNEL);
    if (!k_processes_buffer) {
        return -ENOMEM;
    }

    // Llenar el buffer con las entradas de la lista
    list_for_each_entry(entry, &memory_limitation_head, list) {  
        if (count >= max_entries) {
            break;
        }
        k_processes_buffer[count].pid = entry->node.pid;  
        k_processes_buffer[count].memory_limit = entry->node.memory_limit;  
        count++;
    }

    // Copiar el buffer al espacio de usuario
    if (copy_to_user(u_processes_buffer, k_processes_buffer, count * sizeof(struct memory_limitation))) {
        kfree(k_processes_buffer);
        return -EFAULT;
    }

    // Liberar memoria y establecer processes_buffer_size
    kfree(k_processes_buffer);

    if (put_user(count, processes_buffer_size)) {
        return -EFAULT;
    }

    return 0; // Éxito
}
```

Para esta syscall si se profundizará más acerca de su funcionamiento.

#### 1. **Validación de los parámetros de entrada**

- **`max_entries`**:
  - Este parámetro indica la cantidad máxima de entradas que el usuario espera recibir.
  - Si su valor es menor o igual a 0, la syscall devuelve `-EINVAL` porque no tendría sentido procesar una solicitud sin espacio suficiente para almacenar los datos.
- **Verificación de punteros del espacio de usuario (`u_processes_buffer` y `processes_buffer_size`)**:
  - Se usa `access_ok` para validar que ambos punteros apuntan a regiones válidas en el espacio de usuario.
  - Si alguno de los punteros es inválido, se devuelve `-EINVAL` para evitar violaciones de acceso.

---

#### 2. **Reserva de memoria en el espacio del kernel**

- Se reserva un buffer en memoria del kernel utilizando `kmalloc_array`. Este buffer se utilizará para almacenar temporalmente los datos de la lista enlazada antes de copiarlos al espacio de usuario.
- Si no hay suficiente memoria disponible, se devuelve `-ENOMEM`, indicando que ocurrió un error de asignación.

---

#### 3. **Iteración sobre la lista de procesos**

- Se utiliza `list_for_each_entry` para recorrer la lista enlazada `memory_limitation_head`. Por cada nodo de la lista:
  - Se verifica que no se exceda el número máximo de entradas permitidas (`max_entries`). Si se alcanza el límite, se detiene la iteración.
  - Los datos de cada proceso (`pid` y `memory_limit`) se copian al buffer del kernel (`k_processes_buffer`).

---

#### 4. **Copia al espacio de usuario**

- Los datos acumulados en el buffer del kernel se copian al buffer del usuario (`u_processes_buffer`) utilizando `copy_to_user`.
  - Si ocurre un error durante esta copia (por ejemplo, si el puntero del usuario apunta a una dirección no válida), se libera la memoria reservada con `kfree` y se devuelve `-EFAULT` (fallo de acceso).
- Este paso es fundamental porque el kernel y el usuario operan en espacios de memoria separados, por lo que los datos deben transferirse explícitamente.

---

#### 5. **Liberación de memoria y escritura del tamaño del buffer**

- Después de copiar los datos al espacio de usuario, el buffer temporal del kernel se libera con `kfree` para evitar fugas de memoria.
- Se utiliza `put_user` para escribir en el puntero `processes_buffer_size` la cantidad de entradas efectivamente copiadas. Esto permite al programa del usuario saber cuántos procesos se incluyeron en la respuesta.
  - Si falla esta operación, se devuelve `-EFAULT`, indicando un problema al escribir en el espacio de usuario.

## Actualizar el límite de un proceso

```c
SYSCALL_DEFINE2(luis_update_memory_limit, pid_t, process_pid, size_t, memory_limit) {
    struct task_struct *task;
    struct memory_limitation_list *node;
    int ret;

    // Verificar permisos
    if (!capable(CAP_SYS_ADMIN)) {
        return -EPERM;
    }

    // Validar argumentos
    if (process_pid <= 0 || memory_limit <= 0) {
        return -EINVAL; // Argumento inválido
    }

    // Buscar el proceso por PID
    task = find_task_by_vpid(process_pid);
    if (!task) {
        return -ESRCH; // Proceso no encontrado
    }

    // Verificar si el proceso está en la lista
    node = find_node_by_pid(process_pid);
    if (!node) {
        return -102; 
    }

    // Verificar si el uso actual de memoria excede el nuevo límite
    if (get_process_memory_usage(task) > memory_limit) {
        return -100; // Límite excedido
    }

    // Actualizar el límite de memoria con do_prlimit
    ret = set_process_memory_limit(task, memory_limit);
    if (ret) {
        return ret; // Error al establecer el límite
    }

    // Actualizar el límite en la lista
    node->node.memory_limit = memory_limit;

    return 0; // Éxito
}
```

- **Verificación de permisos y validación de argumentos**:
  
  - Usa `capable(CAP_SYS_ADMIN)` para asegurarse de que el usuario tenga privilegios de administrador (`root`).
  - Valida que el `process_pid` y el `memory_limit` sean valores positivos. Si alguno de los dos no es válido, la syscall retorna `-EINVAL` (argumento inválido).

- **Búsqueda del proceso y verificación en la lista**:
  
  - Se utiliza `find_task_by_vpid` para buscar el proceso por su PID.
  - Luego, se comprueba si el proceso está registrado en la lista de limitaciones de memoria con `find_node_by_pid`. Si no está, retorna `-102` (proceso no encontrado).

- **Verificación del uso actual de memoria**:
  
  - Obtiene el uso de memoria actual del proceso con `get_process_memory_usage`.
  - Si el uso actual de memoria excede el nuevo límite propuesto, la syscall retorna `-100` (límite excedido).

- **Actualización del límite de memoria**:
  
  - Si todo es válido, se actualiza el límite de memoria del proceso utilizando `set_process_memory_limit`.
  - Finalmente, se actualiza el valor del límite de memoria en la lista (`memory_limitation_list`) para reflejar el cambio.

## Remover el límite de un proceso

```c
SYSCALL_DEFINE1(luis_remove_memory_limit, pid_t, process_pid) {
    struct task_struct *task;
    struct memory_limitation_list *node;
    struct rlimit unlimited_rlimit;
    int ret;

    // Verificar permisos
    if (!capable(CAP_SYS_ADMIN)) {
        return -EPERM;
    }

    // Validar el argumento del PID
    if (process_pid <= 0) {
        return -EINVAL;
    }

    // Buscar el proceso por PID
    task = find_task_by_vpid(process_pid);
    if (!task) {

        return -ESRCH; // Proceso no encontrado
    }

    // Verificar si el proceso está en la lista
    node = find_node_by_pid(process_pid);
    if (!node) {
        return -102;
    }


    ret = set_process_memory_limit(task, 110 * 1024 * 1024); 
    if (ret) {
        return ret;
    }


    list_del(&node->list);
    kfree(node);

    return 0; // Éxito
}
```

**Verificación de permisos**:

- Utiliza la función `capable(CAP_SYS_ADMIN)` para comprobar si el usuario tiene privilegios de administrador (`root`).
- Si no tiene permisos, la syscall devuelve `-EPERM` (operación no permitida).

**Validación del PID**:

- Se valida si el `process_pid` es mayor que 0. Si es un valor no válido, devuelve `-EINVAL` (argumento inválido).

**Búsqueda del proceso y verificación en la lista**:

- Se utiliza `find_task_by_vpid(process_pid)` para encontrar el proceso correspondiente al PID. Si el proceso no se encuentra, devuelve `-ESRCH` (proceso no encontrado).
- Luego, se busca si el proceso está en la lista de limitaciones de memoria con `find_node_by_pid(process_pid)`. Si no está en la lista, devuelve `-102`.

**Eliminación del nodo de la lista**:

- Utiliza `list_del(&node->list)` para eliminar el nodo correspondiente del encabezado de la lista `memory_limitation_head`.
- Luego, se libera la memoria del nodo con `kfree(node)`.
