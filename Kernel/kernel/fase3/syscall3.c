#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/syscalls.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/list.h>
#include <linux/fs.h>
#include <linux/capability.h>
#include <syscall3.h>
struct memory_limitation {
    pid_t pid;
    size_t memory_limit;
};

struct memory_limitation_list {
    struct list_head list; 
    struct memory_limitation node; 
};


static LIST_HEAD(memory_limitation_head);

// Función para establecer el límite de memoria de un proceso
static int set_process_memory_limit(struct task_struct *task, size_t memory_limit) {
    struct rlimit new_rlimit;

    new_rlimit.rlim_cur = memory_limit;
    new_rlimit.rlim_max = memory_limit;

    return do_prlimit(task, RLIMIT_AS, &new_rlimit, NULL);
}

// Función para buscar un nodo por PID
static struct memory_limitation_list *find_node_by_pid(pid_t pid) {
    struct memory_limitation_list *entry;

    list_for_each_entry(entry, &memory_limitation_head, list) {
        if (entry->node.pid == pid) {
            return entry; // Nodo encontrado
        }
    }

    return NULL; // Nodo no encontrado
}

// Función para obtener el uso actual de memoria del proceso
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

   
    new_entry->node.pid = pid;
    new_entry->node.memory_limit = memory_limit;
    INIT_LIST_HEAD(&new_entry->list);

    // Agregar a la lista principal
    list_add(&new_entry->list, &memory_limitation_head);

    return 0; // Nodo agregado exitosamente
}


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

    // Buscar el proceso por PID
    task = find_task_by_vpid(process_pid);
    if (!task) {
        return -ESRCH; 
    }

    // Verificar si ya existe el nodo en la lista
    existing_node = find_node_by_pid(process_pid);
    if (existing_node) {
        return -101; 
    }

    // Verificar el uso actual de memoria
    current_memory_usage = get_process_memory_usage(task);
    if (current_memory_usage > memory_limit) {
        return -100; 
    }

    // Establecer el límite de memoria
    ret = set_process_memory_limit(task, memory_limit);
    if (ret) {
        return ret; 
    }

    return add_memory_limitation_node(process_pid, memory_limit);
}



SYSCALL_DEFINE3(luis_get_memory_limits, struct memory_limitation*, u_processes_buffer, size_t, max_entries, int*, processes_buffer_size) {
    struct memory_limitation_list *entry; 
    int count = 0;

    
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

   
    if (copy_to_user(u_processes_buffer, k_processes_buffer, count * sizeof(struct memory_limitation))) {
        kfree(k_processes_buffer);
        return -EFAULT;
    }

    
    kfree(k_processes_buffer);

    if (put_user(count, processes_buffer_size)) {
        return -EFAULT;
    }

    return 0; 
}

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
        return -EINVAL; 
    }

    // Buscar el proceso por PID
    task = find_task_by_vpid(process_pid);
    if (!task) {
        return -ESRCH; 
    }

    // Verificar si el proceso está en la lista
    node = find_node_by_pid(process_pid);
    if (!node) {
        return -102; 
    }

    // Verificar si el uso actual de memoria excede el nuevo límite
    if (get_process_memory_usage(task) > memory_limit) {
        return -100; 
    }

    // Actualizar el límite de memoria con do_prlimit
    ret = set_process_memory_limit(task, memory_limit);
    if (ret) {
        return ret; 
    }

    // Actualizar el límite en la lista
    node->node.memory_limit = memory_limit;

    return 0; 
}



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
        
        return -ESRCH; 
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

    return 0; 
}






