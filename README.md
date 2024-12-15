# SO2-202003745-VD2024

## Descripción del proyecto

En el ámbito de los sistemas operativos, la capacidad de personalizar y expandir el kernel permite a los desarrolladores ajustar el funcionamiento del sistema a necesidades específicas.
Este proyecto aborda el desarrollo de nuevas funcionalidades en el kernel de Linux, brindando la oportunidad de entender su estructura y funcionamiento interno, además de aprender a trabajar con módulos y llamadas al sistema personalizadas.

Las personalizaciones estarán limitadas a aspectos específicos como la modificación
del nombre del sistema, mensajes de arranque y el desarrollo de módulos para obtener estadísticas del sistema. No se espera una modificación extensa del sistema operativo ni el desarrollo de nuevas arquitecturas o drivers avanzados.

## Requisitos del proyecto

- Entorno de ejecución para máquinas virtuales

- Linux mint 

## Configuración del entorno para la realización del proyecto

Para poder realizar este proyecto, se hizo uso de una máquina virtual, la cual tiene linux mint (versión cinamon), a continuación se explicará cuál es la mejor configuración para poder realizar este proyecto correctamente.

#### Configuración para la maquina virtual

Se recomienda darle los siguientes requisitos a la máquina virtual:

<img title="" src="assets/2024-12-11-17-09-06-image.png" alt="" width="456" data-align="inline">

<img src="assets/2024-12-11-17-10-08-image.png" title="" alt="" width="459">

Para evitar cualquier inconveniente también se recomienda darle un tamaño de 40gb, así podremos compilar el kernel perfectamente. 

#### Versión del kernel

Para realizar este proyecto se descargo la versión *6.8*, la cual fue descargada en kernel.org.

#### Recomendación para copiar y pegar

Para poder copiar entre nuestra máquina host y anfitrion, existen dos opciones.

- Conectase mediante ssh.

- Instalar guest additions de vbox.

# Compilar el kernel (Si es por primera vez)

## Descargar y descomprimir el código fuente del kernel

```bash
wget https://cdn.kernel.org/pub/linux/kernel/v6.x/linux-6.8.tar.xz
```

Descargar el archivo del kernel desde `kernel.org`.

```bash
tar -xvf linux-6.8.tar.xz
```

Descomprimir el archivo descargado.

---

## Instalar las dependencias necesarias

```bash
sudo apt-get install build-essential libncurses5-dev fakeroot wget bzip2 openssl
sudo apt-get install build-essential libncurses-dev bison flex libssl-dev libelf-dev
```

Instalar las herramientas y librerías requeridas para compilar el kernel.

---

## Preparar el entorno de compilación

Ejecutar los siguientes comandos en el directorio del código fuente del kernel como usuario root:

### Copiar el archivo de configuración existente

```bash
cp -v /boot/config-$(uname -r) .config
```

Copiar la configuración actual del kernel al directorio de compilación.

### Limpiar el ambiente de compilación

```bash
make clean
```

Eliminar archivos generados previamente en caso de haberlos.

### Modificar la versión del kernel en el archivo Makefile

Editar el archivo `Makefile` en la raíz del directorio del código fuente y modificar la línea:

```make
EXTRAVERSION =
```

Cambiándola a algo como:

```make
EXTRAVERSION = -49-usac1
```

Esto agrega una etiqueta personalizada a la versión del kernel.

---

## Configurar el kernel

### Crear la configuración inicial basada en el archivo copiado

```bash
make oldconfig
```

Se recomienda presionar **Enter** en todas las opciones que aparezcan.

### Crear configuraciones locales para los módulos

```bash
make localmodconfig
```

Esto optimiza el kernel configurando sólo los módulos necesarios para el hardware presente en la máquina.

### Deshabilitar certificados de firma oficiales de Canonical

```bash
scripts/config --disable SYSTEM_TRUSTED_KEYS
scripts/config --disable SYSTEM_REVOCATION_KEYS
```

Esto deshabilita las claves de firma que podrían generar problemas durante la compilación.

---

## Compilar e instalar el kernel

### Crear un script para compilar e instalar

Ejecutar el archivo `compile_and_install.sh` 

Marcar el archivo como ejecutable:

```bash
chmod +x compile_and_install.sh
```

Ejecutar el script:

```bash
./compile_and_install.sh
```

Presionar **Enter** en todas las opciones de configuración que aparezcan durante el proceso.

---

## Finalizar la instalación

Si el script no actualiza automáticamente la configuración del gestor de arranque, ejecutar:

```bash
sudo update-grub2
```

Reiniciar la máquina virtual. Mantener presionada la tecla **Shift** al iniciar para acceder al menú del gestor de arranque y seleccionar el nuevo kernel.

---

## Debug

Si el kernel no aparece en la lista del gestor de arranque o no inicia, intentar:

```bash
sudo update-grub2
```

## 1.2 Reecompilar el kernel

Para volver a compilar el kernel, no es necesario volver a realizar todos los pasos, lo único para volver a compilar el kernel, cuando hayamos hecho cambios, solo debe ejecutarse el archivo compile.sh.

# 2. Código fuente del kernel modificado

## 2.1 Personalización del nombre del sistema

Para la personalización del nombre del sistema, es necesario irse al siguiente archivo: 

```
/home/luis/Escritorio/linux-6.8/include/linux
```

Una vez en ese archivo, mostrará el siguiente contenido:

```C
/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_UTS_H
#define _LINUX_UTS_H

/*
 * Defines for what uname() should return 
 */
#ifndef UTS_SYSNAME
#define UTS_SYSNAME "Linux"
#endif

#ifndef UTS_NODENAME
#define UTS_NODENAME CONFIG_DEFAULT_HOSTNAME /* set by sethostname() */
#endif

#ifndef UTS_DOMAINNAME
#define UTS_DOMAINNAME "(none)"    /* set by setdomainname() */
#endif

#endif
```

Para cambiar el nombre debemos cambiar la siguiente línea:

```
#define UTS_SYSNAME "Linux"
```

al nombre que queremos cambiar, por ejemplo:

```
#define UTS_SYSNAME "Linux SO2_USAC"
```

Una vez realizado estos pasos, volviendo a recompilar debe salirnos el nombre del sistema al que cambiamos.

## 2.2 Mensajes personalizados

Para poder mostrar mensajes personalizados en nuestro kernel al momento del arranque del sistema debemos modificar el archivo que se encuentra en la siguiente ruta:

```
/home/luis/Escritorio/linux-6.8/init/main.c
```

Una vez en este archivo, buscaremos la función start_kernel, una vez hayamos encontrado esa función, modificaremos lo que queremos que nos salga al inicio, por ejemplo debería de quedar una función como se muestra en la siguiente imagen:

<img src="assets/2024-12-11-21-48-28-image.png" title="" alt="" width="570">

Una vez realizado estos pasos, con el comando 

```bash
dmesg 
```

nos debe salir al inicio el mensaje personalizado que introdujimos, de la siguiente manera:

<img src="assets/2024-12-11-22-02-06-image.png" title="" alt="" width="567">

# 3. Implementación de nuevas syscalls

Mediante el uso de las syscalls, se implementaron nuevas llamadas, las cuales son las siguientes:

- capture_memory_snapshot

- track_syscall_usage

- get_io_throttle

A continuación se describirá el funcionamiento de cada una, pero antes se dará una breve explicación de como es su implementación en general:

## 3.1 Como implementar las syscalls

### 3.1.1 Modificar el archivo syscall64.tbl

En este archivo veremos todas las llamadas al sistema, después de ver los comentarios procederemos a agregar nuestras llamadas para que queden de la siguiente manera:

![](assets/2024-12-14-12-46-06-image.png)

Estas llamadas se desglosan en 3 parametros que son los siguientes:

- **Número de syscall**: El número (como `548`, `549`, `550`) es un identificador único para cada syscall, usado por el sistema operativo para saber qué función ejecutar en el kernel cuando se invoca desde el espacio de usuario.

- **`common`**: La palabra `common` indica que la syscall es común a todas las arquitecturas soportadas, no específica de una arquitectura (como `x86_64` o `arm64`), por lo que está disponible en múltiples plataformas.

- **Dos nombres**: El primer nombre (como `luis_capture_memory_snapshot`) es el nombre accesible desde el espacio de usuario, mientras que el segundo nombre (como `sys_luis_capture_memory_snapshot`) es el nombre interno en el kernel, siguiendo la convención de prefijar con `sys_` para funciones de syscall.

### 3.1.2. Modificar el archivo syscalls.h

En el archivo `syscalls.h`, se definen las declaraciones de las funciones de las syscalls del kernel, como:

![](assets/2024-12-14-12-53-05-image.png)

- **`asmlinkage`**: Es un modificador que indica cómo se debe hacer la llamada a la función en el contexto de ensamblador y el sistema de llamadas al kernel. Garantiza que los parámetros se pasen de manera adecuada al kernel desde el espacio de usuario.

- **`long`**: Tipo de retorno de la syscall, que generalmente indica un valor de estado o error.

- **`__user`**: Especifica que los punteros son del espacio de usuario, y se utiliza para asegurar que los datos se copien correctamente entre el espacio de usuario y el del kernel.

### 3.1.3 Realizar nuestras definiciones en el sys.c

En el archivo sys.c procederemos a realizar nuestras definiciones, un ejemplo sería el siguiente: 

```c
#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/uaccess.h>

SYSCALL_DEFINE1(get_system_memory_size, unsigned long __user *, user_memory_size)
{
    unsigned long memory_size;


    memory_size = totalram_pages() * PAGE_SIZE;


    if (!access_ok(user_memory_size, sizeof(memory_size)))
        return -EFAULT;


    if (copy_to_user(user_memory_size, &memory_size, sizeof(memory_size)))
        return -EFAULT;

    return 0; 
}
```

Este es un ejemplo base, no es una definición de las syscalls descritas anteriormente, los puntos importantes en este ejemplo son los siguientes:

**`SYSCALL_DEFINE1`**: Macro usada para definir una syscall que toma un argumento. El número `1` indica que esta syscall tiene un argumento.

- `get_system_memory_size`: Es el nombre de la syscall.
- `unsigned long __user *, user_memory_size`: El argumento que toma la syscall, un puntero al espacio de usuario donde se almacenará el tamaño de la memoria.
- **`access_ok` y `copy_to_user`**: Son funciones del kernel que se aseguran de que los punteros de usuario sean válidos y luego copian los datos desde el kernel al espacio de usuario.

## 3.2 Explicacion de las syscalls y sus definiciones

### 3.2.1 Capture_memory_snapshot

Esta llamada accede a la tabla de páginas del sistema y devuelve detalles estructurados del uso de memoria (páginas activas, caché, swap, etc.). Para esta implementación, los estudiantes deben entender el manejo de tablas de páginas dentro del kernel y desarrollar estructuras que recopilen y organicen esta información en un formato fácil de interpretar.

##### Definición en sys.c

En sys.c se implemento lo siguiente:

```c
struct memory_snapshot {
    unsigned long total_mem_kb;
    unsigned long free_mem_kb;
    unsigned long cached_mem_kb;  // Memoria en caché
    unsigned long dirty_page_count;  // Páginas sucias
    unsigned long slab_mem_kb;  // Memoria slab total
    unsigned long reclaimable_slab_mem_kb;  // Slab que puede reclamarse
    unsigned long active_page_count;
    unsigned long inactive_page_count;
    unsigned long total_swap_kb;
    char snapshot_date[36]; 
};

SYSCALL_DEFINE1(luis_capture_memory_snapshot, struct memory_snapshot __user *, user_snapshot)
{
    struct memory_snapshot local_snapshot;
    struct sysinfo sys_info;
    struct timespec64 ts;
    struct tm tm;


    if (!user_snapshot)
        return -EINVAL;


    si_meminfo(&sys_info);

    local_snapshot.total_mem_kb = sys_info.totalram << (PAGE_SHIFT - 10);
    local_snapshot.free_mem_kb = sys_info.freeram << (PAGE_SHIFT - 10);
    local_snapshot.cached_mem_kb = sys_info.bufferram << (PAGE_SHIFT - 10); // Usar bufferram como caché
    local_snapshot.total_swap_kb = sys_info.totalswap << (PAGE_SHIFT - 10);


    local_snapshot.active_page_count = global_node_page_state(NR_ACTIVE_ANON) +
                                       global_node_page_state(NR_ACTIVE_FILE);
    local_snapshot.inactive_page_count = global_node_page_state(NR_INACTIVE_ANON) +
                                         global_node_page_state(NR_INACTIVE_FILE);
    local_snapshot.dirty_page_count = global_node_page_state(NR_FILE_DIRTY);


    local_snapshot.slab_mem_kb = global_node_page_state(NR_SLAB_UNRECLAIMABLE_B) << (PAGE_SHIFT - 10);
    local_snapshot.reclaimable_slab_mem_kb = global_node_page_state(NR_SLAB_RECLAIMABLE_B) << (PAGE_SHIFT - 10);


    ktime_get_real_ts64(&ts);
    time64_to_tm(ts.tv_sec, 0, &tm);
    snprintf(local_snapshot.snapshot_date, sizeof(local_snapshot.snapshot_date),
         "%04ld-%02d-%02d %02d:%02d:%02d UTC",
         tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
         tm.tm_hour, tm.tm_min, tm.tm_sec);

    // Copiar los datos al espacio de usuario
    if (copy_to_user(user_snapshot, &local_snapshot, sizeof(local_snapshot)))
        return -EFAULT;

    return 0;
}
```

Explicación del funcionamiento y sus puntos más importantes:

- **Estructura de la Syscall**
  
  - **Entrada del usuario:** La syscall recibe un puntero de tipo `struct memory_snapshot` que se llenará con datos del sistema.
  - **Validación inicial:** Si el puntero proporcionado es inválido (`NULL`), retorna el error `-EINVAL`.
  - **Uso de `si_meminfo`:** Obtiene información global del sistema, como memoria total (`totalram`), memoria libre (`freeram`), y swap total (`totalswap`), y los convierte a kilobytes utilizando `PAGE_SHIFT`.

- **Cálculo de métricas**
  
  - **Páginas activas e inactivas:** Usando `global_node_page_state` con contadores `NR_*`, como:
    - `NR_ACTIVE_ANON` y `NR_ACTIVE_FILE` para páginas activas.
    - `NR_INACTIVE_ANON` y `NR_INACTIVE_FILE` para páginas inactivas.
  - **Páginas sucias:** `NR_FILE_DIRTY` indica el número de páginas modificadas que aún no se escriben en disco.
  - **Memoria slab:** Se divide en:
    - `NR_SLAB_UNRECLAIMABLE_B`: Memoria slab no recuperable.
    - `NR_SLAB_RECLAIMABLE_B`: Memoria slab que puede ser liberada.

- **Uso de `PAGE_SHIFT`**
  
  - `PAGE_SHIFT` representa el tamaño de las páginas de memoria en bits (por ejemplo, 12 para páginas de 4 KB).
  - Al desplazar `PAGE_SHIFT - 10`, se convierte de páginas a kilobytes:
    - Ejemplo: `sys_info.totalram << (PAGE_SHIFT - 10)` multiplica el número de páginas por 4 para obtener el tamaño en KB.

- **Generación de la fecha del snapshot**
  
  - Usa `ktime_get_real_ts64` para obtener el tiempo actual del sistema en segundos.
  - Convierte el tiempo a una estructura de fecha/hora (`tm`) con `time64_to_tm`.
  - Formatea la fecha como una cadena con `snprintf`.

- **Copia al espacio de usuario**
  
  - Copia los datos recolectados al espacio de usuario utilizando `copy_to_user`.

### 3.2.2 Implementación de track_syscall_usage

Se debera modificar la tabla de llamadas del sistema y realizar un rastreo de cada una de las llamadas seleccionadas (open, read, write, fork). La implementación incluye interceptar estas llamadas en tiempo de ejecución, contabilizar el número de veces que se llaman y almacenar la información en estructuras internas que pueden consultarse.

#### Implementación en sys.c

```c
struct track_counters {
    long counter_open;
    long counter_read;
    long counter_write;
    long counter_fork;
};

SYSCALL_DEFINE1(luis_track_syscall_usage, struct track_counters __user *, trackers)
{
    struct track_counters local_trackers;

    local_trackers.counter_write = write_call_counter;
    local_trackers.counter_read = read_call_counter;
    local_trackers.counter_open = open_call_counter;
    local_trackers.counter_fork = fork_call_counter;


    if (copy_to_user(trackers, &local_trackers, sizeof(local_trackers)))
        return -EFAULT;

    return 0;
};
```

- **Parámetro de entrada:**  
  Recibe un puntero de tipo `struct track_counters` que apunta a una estructura en el espacio de usuario para almacenar los contadores.

- **Inicialización de datos locales:**  
  Se crea una instancia local de `struct track_counters` llamada `local_trackers`, donde se almacenarán los valores de los contadores del kernel.

- **Asignación de contadores:**  
  Los campos de `local_trackers` son llenados con los valores actuales de contadores globales (como `write_call_counter`, `read_call_counter`, etc.).
  
  - Estos contadores son variables globales que se actualizan en otras partes del kernel cuando las correspondientes syscalls son ejecutadas.
  
  - Estos contadores dependiendo la llamada se actualizan en open.c, read_write.c y fork.c, un ejemplo de la implementación de uno de estos es el siguiente:
  
  - ```c
    long open_call_counter = 0;  // Variable global para el contador
    EXPORT_SYMBOL(open_call_counter);
    long do_sys_open(int dfd, const char __user *filename, int flags, umode_t mode)
    {
        open_call_counter++; // aumento del contador
        struct open_how how = build_open_how(flags, mode);
        return do_sys_openat2(dfd, filename, &how);
    }
    ```

### 3.2.3 Implementación del get_io_throttle

Este código define una syscall llamada `luis_get_io_throttle`, cuyo objetivo es capturar estadísticas de I/O (entrada/salida) de procesos activos en el sistema.

```c
struct io_stats {
    pid_t pid;  
    char comm[18];  
    unsigned long long bytes_read;  
    unsigned long long bytes_written;  
    unsigned long long cancelled_write_bytes;  
    unsigned long long io_wait_time;  
};

struct all_io_stats {
    struct io_stats stats[MAX_PROCS];
    int num_procs;
};

SYSCALL_DEFINE1(luis_get_io_throttle, struct all_io_stats __user *, user_stats)
{
    struct task_struct *task;
    struct all_io_stats kernel_stats;
    int i = 0;

    rcu_read_lock();
    for_each_process(task) {

        unsigned long long read_bytes = task->ioac.read_bytes;
        unsigned long long write_bytes = task->ioac.write_bytes;


        if (read_bytes == 0 && write_bytes == 0) {
            continue;
        }

        if (i >= MAX_PROCS) {
            break;
        }


        kernel_stats.stats[i].pid = task->pid;
        strncpy(kernel_stats.stats[i].comm, task->comm, sizeof(kernel_stats.stats[i].comm) - 1);
        kernel_stats.stats[i].comm[sizeof(kernel_stats.stats[i].comm) - 1] = '\0';  
        kernel_stats.stats[i].bytes_read = read_bytes;
        kernel_stats.stats[i].bytes_written = write_bytes;
        kernel_stats.stats[i].cancelled_write_bytes = task->ioac.cancelled_write_bytes;
        kernel_stats.stats[i].io_wait_time = task->se.sum_exec_runtime - task->se.vruntime;
        i++;
    }
    kernel_stats.num_procs = i;
    rcu_read_unlock();


    if (!access_ok(user_stats, sizeof(kernel_stats))) {
        return -EFAULT;
    }

    // Copiar los datos al espacio de usuario
    if (copy_to_user(user_stats, &kernel_stats, sizeof(kernel_stats))) {
        return -EFAULT;
    }

    return 0;
}
```

### **Funcionamiento de la syscall**

1. **Inicio de la syscall (`SYSCALL_DEFINE1`):**
   
   - **Parámetro de entrada:**  
     Recibe un puntero `user_stats` a una estructura `all_io_stats` ubicada en el espacio de usuario, donde se copiarán los datos.

2. **Bloqueo de lectura de RCU (`rcu_read_lock`):**
   
   - Asegura que el acceso a la lista de procesos sea seguro durante la iteración.

3. **Iteración sobre procesos (`for_each_process`):**
   
   - Recorre todos los procesos activos en el sistema mediante la lista global de procesos (`task_struct`).

4. **Obtención de estadísticas por proceso:**
   
   - Se obtienen los bytes leídos y escritos del proceso (`task->ioac.read_bytes` y `task->ioac.write_bytes`).
   - Si ambos valores son 0, el proceso es ignorado, ya que no ha realizado operaciones de I/O.
   - Los datos son almacenados en `kernel_stats.stats[i]`:
     - `pid` y `comm`: ID y nombre del proceso.
     - `bytes_read`, `bytes_written`, y `cancelled_write_bytes`: Estadísticas relacionadas con el uso de I/O.
     - `io_wait_time`: Calculado como la diferencia entre `sum_exec_runtime` (tiempo total de ejecución) y `vruntime` (tiempo virtual ejecutado). Esto representa el tiempo total que el proceso esperó en operaciones de I/O.

5. **Límite máximo de procesos (`MAX_PROCS`):**
   
   - La syscall se detiene si se alcanza este límite para evitar desbordamientos del arreglo.

6. **Liberación del bloqueo de RCU (`rcu_read_unlock`):**
   
   - Finaliza la sección protegida después de completar la iteración.