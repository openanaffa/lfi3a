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
