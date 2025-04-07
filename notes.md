

- Repositorio en github con el agente en C, tiene que correr dentro de las VMs
    https://github.com/brodriguez-opennebula/get_proc_ticks
- Paquete oficial de scaphandre (from github)
  https://github.com/hubblo-org/scaphandre/releases
  - Servicio systemd para scaphandre (copiarlo de los hosts que ya esta instalado - revisar si esta con docker)
  - Editar fichero de config con SCAPHANDRE_PORT
- Dashboard Grafana
  - En la vista de VM hay una grafica nueva (revisar de donde podemos recuperar el fuente)
- Modificar host y añadir variable SCAPHANDRE_PORT como variable en la config del host 
    SCAPHANDRE_PORT="9930"
- Obtener power metrics por proceso
  - Tener agente qemu corriendo Y permitiendo ejecución de comandos

