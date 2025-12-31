# LFI3A Standard Library

LFI3A comes with a set of built-in functions to handle basic I/O and utility tasks.

## I/O Functions

### `kteb(...)`

Prints the given arguments to the console.

**Example:**

```lfi3a
kteb("Salam", 123)
```

### `wrack_3la()`

Waits for a single key press and returns its ASCII code. Useful for game input.

**Example:**

```lfi3a
dir key = wrack_3la()
ila (key == 119) { // 'w'
    kteb("Moving Up")
}
```

## Utility Functions

### `tul(item)`

Returns the length of a string or an array.

**Example:**

```lfi3a
dir s = "Salam"
kteb(tul(s)) // 5
```

### `naw3(item)`

Returns the type of the item as a string (e.g., "number", "string", "bool", "array", "map").

**Example:**

```lfi3a
kteb(naw3(123)) // "number"
```

### `ra9m(item)`

Converts the item to a number if possible.

**Example:**

```lfi3a
dir s = "123"
dir n = ra9m(s)
kteb(n + 1) // 124
```

### `kelma(item)`

Converts the item to a string.

**Example:**

```lfi3a
dir n = 123
dir s = kelma(n)
kteb(s) // "123"
```

### `wa9t()`

Returns the current time in seconds or similar timestamp depending on the implementation.

**Example:**

```lfi3a
dir current = wa9t()
kteb(current)
```

### `na3ess(ms)`

Sleeps for the specified number of milliseconds.

**Example:**

```lfi3a
na3ess(1000) // Sleep for 1 second
```

## Advanced Math & 3D Support

### Advanced Math

- **`asgher(a, b)`**: Minimum of two values.
- **`akber(a, b)`**: Maximum of two values.
- **`logarithm(n)`**: Natural logarithm ($\ln n$).
- **`as(n)`**: Exponential ($e^n$).
- **`atan2(y, x)`**: Multi-quadrant inverse tangent.
- **`dwer(n)`**: Round to nearest integer.
- **`ls9ef(n)`**: Ceiling (round up).
- **`l9a3(n)`**: Floor (round down).
- **`jdr(n)`**: Square root.
- **`os(base, exp)`**: Power ($base^{exp}$).
- **`3chwa2i()`**: Random number between 0 and 1.
- **`dwer(n)`**: Round to nearest integer.

### Vector Algebra (Mowajeha)

- **`mowajeha3(x, y, z)`**: Creates a 3D vector.
- **`mo_dorbat(v1, v2)`**: Dot product.
- **`mo_ti9ati(v1, v2)`**: Cross product.
- **`mo_toul(v)`**: Vector magnitude (length).
- **`mo_nidam(v)`**: Normalize vector (magnitude = 1).

### Matrix Operations (Masfofa - 4x4)

*Matrices are represented as 1D arrays of 16 numbers.*

- **`masfofa4(...)`**: Creates a 4x4 matrix from 16 values.
- **`mf_mawjud()`**: Identity matrix.
- **`mf_dorbat(m1, m2)`**: Matrix multiplication.
- **`mf_translate(m, v)`**: Translation by vector `v`.
- **`mf_rotate(m, angle, axis)`**: Rotation around `axis` by `angle`.
- **`mf_scale(m, v)`**: Scaling by vector `v`.

### Geometry Helpers

- **`masafa(v1, v2)`**: Distance between two points.
- **`zawiya(v1, v2)`**: Angle between two vectors in radians.
- **`ta9rib(a, b, t)`**: Linear interpolation (lerp).
- **`7essar(v, min, max)`**: Clamps value between min/max.
- **`in3ikas(v, n)`**: Reflects vector `v` against normal `n`.

## Console Graphics & Drawing

Functions for creating TUI (Text User Interface) applications and games.

### `chacha_imsah()`

Clears the terminal screen and resets cursor position.

### `chacha_sedd()`

Clears the screen and shows cursor (cleanup).

### `chacha_3red()`

Flushes the output buffer to screen. Essential for smooth rendering.

### `rsem_mrabba3(x, y, w, h, color_code)`

Draws a colored rectangle at the specified position.

- `x`, `y`: Position coordinates (0-indexed).
- `w`, `h`: Width and Height.
- `color_code`: Integer for ANSI color (0=reset, 1=red, 2=green, 3=blue, 4=yellow, etc.).

**Example:**

```lfi3a
chacha_imsah()
rsem_mrabba3(10, 5, 2, 2, 1) // Draw red square
chacha_3red()
```

## Data Structures

### `mo3jam()`

Creates a new empty Map (Dictionary). Maps use string keys.

**Example:**

```lfi3a
dir m = mo3jam()
m["name"] = "LFI3A"
kteb(m["name"])
```

## Networking

LFI3A supports both low-level socket operations and high-level HTTP requests.

### HTTP Client

#### `talab(host, path, [port])`

Performs a simple HTTP GET request. Returns a Map containing `body` (string), `headers` (string), `raw` (full response), and `status_line`.

**Example:**

```lfi3a
dir res = talab("example.com", "/")
kteb(res["body"])
```

### Low-Level Sockets

#### `socket_jadid()`

Creates a new TCP socket. Returns a file descriptor (number) or -1 on error.

#### `socket_rabt(fd, host, port)`

Connects a socket to a remote host (Client). Returns true on success.

#### `socket_rbet(fd, port)`

Binds a socket to a local port (Server). Returns true on success.

#### `socket_sma3(fd, backlog)`

Listens for incoming connections. `backlog` defaults to 5.

#### `socket_qbal(fd)`

Accepts an incoming connection. Returns a new file descriptor for the client.

#### `socket_ghayr_mghlo9(fd)`

Sets the socket to non-blocking mode. Returns true on success.

#### `socket_sift(fd, message)`

Sends a string message over the socket.

#### `socket_sta9bel(fd, size)`

Receives up to `size` bytes from the socket. Returns string.

#### `socket_sed(fd)`

Closes the socket.
