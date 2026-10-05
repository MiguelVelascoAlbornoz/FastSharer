# FastSharer

Herramienta de línea de comandos en C++ para compartir rápidamente archivos y carpetas de tu PC a través de un servidor HTTP local. Otros dispositivos de la red pueden abrir una página web generada automáticamente con el listado de archivos y descargarlos desde el navegador.

> Estado: en desarrollo.

## Características

- Comparte una carpeta indicando solo su ruta.
- Genera una página HTML con el árbol de archivos (plantilla en `resources/index.html`, el marcador `{{FILES}}` se sustituye por el listado).
- Soporte de rutas con caracteres especiales (decodificación de URL).
- Plantilla HTML embebida en el ejecutable: no hay que distribuir archivos extra.
- Sistema de build propio incremental (`Mython.py`).

## Requisitos

- Windows (usa `wininet` y `ws2_32`)
- `g++` con soporte para C++17 o superior, disponible en el `PATH` (MinGW-w64 / MSYS2)
- Python 3.8+ (para el script de build)

## Estructura del proyecto

```
FastSharer/
├── src/                 Código fuente (.cpp / .h)
│   ├── Commands.h       Comandos de la CLI (help, ls, start)
│   └── dirGetter.h      Lectura de directorios y generación del HTML
├── resources/           Archivos embebidos en el ejecutable (index.html)
├── external/            Includes y librerías de terceros (includes/, libs/)
├── build/               Generado por el build (no se versiona)
│   ├── compilationFiles/  Objetos .o y .d por modo de compilación
│   ├── generated/         resources.h generado automáticamente
│   └── out/               Ejecutables finales
├── Mython.py            Script de compilación
└── compile_commands.json  Generado por el build (para el IDE)
```

## Compilar

Desde la raíz del proyecto:

```bash
python Mython.py debug      # -g -O0 -D_DEBUG
python Mython.py release    # build de release
```

El ejecutable queda en `build/out/<modo>/FastSharer.exe`.

### Opciones del build

| Opción | Descripción |
|---|---|
| `-e`, `--execute` | Ejecuta el programa al terminar de compilar |
| `programa_args...` | Argumentos que se pasan al ejecutable (junto con `-e`) |

Ejemplo:

```bash
python Mython.py debug -e start C:\Users\yo\Compartir
```

### Cómo funciona el build

- **Compilación incremental**: solo se recompilan los `.cpp` modificados o cuyos headers (según los `.d` generados con `-MMD -MP`) cambiaron.
- **Resources**: todo archivo dentro de `resources/` se convierte en un array de bytes en `build/generated/resources.h`. Para el archivo `index.html` se generan `INDEX_DATA` e `INDEX_SIZE`.
- **Warnings como errores**: se compila con `-Wall -Wextra -Werror`.
- Se genera `compile_commands.json` para CLion y otros IDEs.
- Los `.o`/`.d` de archivos eliminados del proyecto se borran automáticamente.

## Uso

```
FastSharer help            Muestra la ayuda
FastSharer ls              Lista los archivos
FastSharer start <ruta>    Inicia el servidor compartiendo <ruta>
```

Una vez iniciado, abre en el navegador la dirección que muestre el programa (por ejemplo `http://<tu-ip>:<puerto>`) desde cualquier dispositivo de la misma red.

> Completa esta sección con el puerto por defecto y cualquier flag adicional que añadas.

## Seguridad

- Comparte únicamente carpetas que quieras hacer accesibles a otros equipos de tu red.
- Windows puede pedir permiso en el firewall la primera vez que se inicia el servidor.
- No expongas el puerto a Internet.

## Hoja de ruta

- [ ] Instalador (instalación por usuario, añadir al `PATH`)
- [ ] Puerto configurable
- [ ] Protección contra path traversal (`..`) en las rutas solicitadas
- [ ] Soporte multiplataforma
- [ ] ZIP atravez del sitio web
- [ ] Mostrar cuanto pesa cada arvhivo
- [ ] Mejorar los iconos

## Licencia

Por definir.
