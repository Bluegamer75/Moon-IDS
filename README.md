# Moon IDS



El sistema desarrollado en este momento es una herramienta básica de escaneo de puertos con capacidad limitada para identificar algunos servicios comunes que corren en esos puertos. Actualmente, el IDS está enfocado en realizar las siguientes tareas:

    Escaneo de puertos: Permite escanear un rango de puertos especificado en una dirección IP para determinar si están abiertos o cerrados.

    Identificación de servicios: Para los puertos más comunes (como HTTP, HTTPS, FTP, SSH, Tor y Minecraft), el sistema es capaz de recuperar banners de servicio, lo que ayuda a identificar el servicio que podría estar corriendo en ese puerto.

Objetivos a Futuro:

El sistema tiene como objetivo expandirse en el futuro con las siguientes capacidades adicionales:

    Monitorizar servicios y hosts en tiempo real (realizando escaneos completos y verificando la integridad)

    Seguridad: Realizar escaneos para revisar los cambios del sistemas, sea de archivos o de directorios, leer logs del sistemas criticos y analizarlos

    Analisis de red en tiempo real para detectar posibles ataques DOS o Infiltraciones dentro de las maquinas (log de sudo)


