# LFI3A Syntax Guide

LFI3A syntax is designed to be intuitive for Moroccan Arabic speakers. It uses Darija keywords for all major programming constructs.

## Variables

Use the `dir` (put/set) keyword to declare variables.

```lfi3a
dir ism = "Oussama"
dir age = 22
dir s7i7_bool = s7i7   // true
dir ghalat_bool = ghalat // false
```

## Control Flow

### If Statements

- `ila`: if
- `wila`: else if
- `wla`: else

```lfi3a
ila (score >= 90) {
    kteb("Excellent")
} wila (score >= 50) {
    kteb("Pass")
} wla {
    kteb("Fail")
}
```

### Loops

#### While Loop (`ma7ad`)

```lfi3a
dir i = 0
ma7ad (i < 5) {
    kteb(i)
    i = i + 1
}
```

#### For Loop (`kol`)

```lfi3a
kol (dir i = 0; i < 10; i = i + 1) {
    kteb(i)
}
```

## Functions

Use `dalla` to define a function.

```lfi3a
dalla salam(ism) {
    kteb("Salam", ism)
}

salam("Oussama")
```

## Operators

- **Arithmetics**: `+`, `-`, `*`, `/`
- **Comparison**: `==`, `!=`, `<`, `>`, `<=`, `>=`
- **Logical**: `w` (and), `wla` (or)

## Comments

Use `//` for single-line comments.

```lfi3a
// This is a comment
```
