# AuthServer

**Autor:** Jesús de la Fuente Parrilla  
**Fecha:** 30 de abril de 2026

## Descripción

Implementación de un sistema de autenticación cliente-servidor basado en HMAC-SHA1 y nonce. El servidor genera un nonce aleatorio por conexión, el cliente responde con su login, un timestamp y el HMAC-SHA1 calculado sobre `nonce || timestamp` usando su clave secreta. El servidor verifica el HMAC y el timestamp antes de aceptar la autenticación.

## Archivos

| Fichero | Descripción |
|---|---|
| `authserver.c` | Proceso servidor: escucha conexiones y verifica autenticaciones |
| `authclient.c` | Proceso cliente: se autentica contra el servidor |
| `utils.c` / `utils.h` | Funciones compartidas: HMAC-SHA1, nonce, lectura de cuentas, E/S de socket |
| `accounts.txt` | Base de datos de cuentas en formato `login:clave_hex` |

## Compilación

Requiere `gcc` y la librería OpenSSL.

```bash
make
```

Esto genera los dos binarios: `authserver` y `authclient`.

Para compilar solo uno de ellos:

```bash
make authserver
make authclient
```

Para limpiar los binarios:

```bash
make clean
```

## Uso

### Servidor

```bash
./authserver accounts.txt [puerto]
```

- `accounts.txt` — fichero de cuentas (obligatorio)
- `puerto` — puerto de escucha (opcional, por defecto 9999)

### Cliente

```bash
./authclient login clave_hex dirección_servidor puerto
```

- `login` — nombre de usuario
- `clave_hex` — clave de 20 bytes en hexadecimal (40 caracteres)
- `dirección_servidor` — IP del servidor
- `puerto` — puerto del servidor

### Ejemplo

```bash
./authserver accounts.txt 9999 &
./authclient alice 0a1b2c3d4e5f... 127.0.0.1 9999
```
