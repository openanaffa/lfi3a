# LFI3A Standard Library

LFI3A comes with a set of built-in functions to handle basic I/O and utility tasks.

## I/O Functions

### `kteb(...)`

Prints the given arguments to the console.

**Example:**

```lfi3a
kteb("Salam", 123)
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

Returns the type of the item as a string (e.g., "number", "string", "bool", "array").

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

## Advanced Math & 3D Support

### Advanced Math

- **`asgher(a, b)`**: Minimum of two values.
- **`akber(a, b)`**: Maximum of two values.
- **`logarithm(n)`**: Natural logarithm ($\ln n$).
- **`as(n)`**: Exponential ($e^n$).
- **`atan2(y, x)`**: Multi-quadrant inverse tangent.

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
