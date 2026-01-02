# Filtering (selection) system

Some optional arguments of Voronota-LT expect selection expressions:

```
--restrict-input-atoms                           string     selection expression to restrict input balls
--restrict-contacts                              string     selection expression to restrict contacts before construction
--restrict-contacts-for-output                   string     selection expression to restrict contacts for output
--restrict-atom-descriptors-for-output           string     selection expression to restrict single-index data (balls, cells, sites) for output
```

The expressions need to be specified using the filtering (selection) language described in this section.
The language is used to select
**atoms** or atom-based descriptors (like **cells** and **sites**) and **contacts**.

The language supports explicit boolean logic with round brackets to avoid
any ambiguity in operator precedence.
It is recommended to always use round brackets when combining filters with logical operators.
Even when redundant, over-bracketing makes expressions easier to read and safer for users unfamiliar with precedence rules.

## General syntax

### Filter blocks

A filter block is written in square brackets:

```
[ <clause> <clause> ... ]
```

- Clauses are space-separated.
- Clause arguments are single tokens.
- Lists use commas (`,`).
- Numeric ranges use colons (`:`).

### Boolean expressions

Filter blocks can be combined into boolean expressions.

Supported operators:

| Logical meaning | Accepted forms      |
|-----------------|---------------------|
| AND             | `and`, `&`, `&&`    |
| OR              | `or`, `|`, `||`     |
| NOT             | `not`, `!`          |

Round brackets `(` `)` may be nested freely and are encouraged.

### Examples

```
[ ... ]
```

```
( [ ... ] and [ ... ] )
```

```
( ( [ ... ] ) or ( not [ ... ] ) )
```

```
( ( ( [ ... ] and [ ... ] ) or ( [ ... ] ) ) and ( not [ ... ] ) )
```

The same boolean expression syntax applies to **atom selection** and
**contact selection**.

## Atom (or atom-based descriptor) selection

Atom filters match **atoms** based on chain, residue, atom name, element,
and biochemical category.

### Chain selection

```
-chain <list>        or  -c <list>
-chain-not <list>    or  -c! <list>
```

Examples:

```
[ -chain A ]
[ -c A,B ]
[ -c! C ]
```

### Residue number (ranges use ':')

```
-residue-number <intervals>        or  -rnum <intervals>
-residue-number-not <intervals>    or  -rnum! <intervals>
```

Examples:

```
[ -rnum 42 ]
[ -rnum 10:20 ]
[ -rnum! 1:5 ]
```

### Residue name

```
-residue-name <list>        or  -rname <list>
-residue-name-not <list>    or  -rname! <list>
```

Examples:

```
[ -rname ALA ]
[ -rname ALA,GLY,SER ]
[ -rname! PRO ]
```

### Atom name

```
-atom-name <list>        or  -aname <list>
-atom-name-not <list>    or  -aname! <list>
```

Examples:

```
[ -aname CA ]
[ -aname CA,CB ]
[ -aname! H ]
```

### Element

```
-element <list>        or  -elem <list>
-element-not <list>    or  -elem! <list>
```

Examples:

```
[ -elem C ]
[ -elem N,O ]
[ -elem! H ]
```

### Residue ID (combined identifier)


```
-residue-id <list>        or  -rid <list>
-residue-id-not <list>   or  -rid! <list>
```

Supported formats:

- `42` where `42` is the residue number
- `42/A`, where `A` is the insertion code
- `42|ALA`,  where `ALA` is the residue name
- `42/A|ALA`

Examples:

```
[ -rid 42|ALA ]
[ -rid 101/B ]
```

### Protein-specific macros

These predefined macros expand internally to standard residue and atom sets.

| Macro                 | Description                   |
|-----------------------|-------------------------------|
| `-protein`            | All standard protein residues |
| `-protein-backbone`   | Backbone atoms (N, CA, C, O)  |
| `-protein-sidechain`  | Side-chain atoms              |

Examples:

```
[ -protein ]
[ -protein-backbone ]
[ -protein-sidechain ]
```

### Nucleic-acid–specific macros

These macros select standard nucleic-acid residues and optionally restrict
atoms to backbone or base (side-chain) atoms.

| Macro                     | Description                                   |
|---------------------------|-----------------------------------------------|
| `-nucleic`                | All nucleic acids (DNA and RNA)               |
| `-nucleic-dna`            | DNA residues only                             |
| `-nucleic-rna`            | RNA residues only                             |
| `-nucleic-backbone`       | Sugar-phosphate backbone atoms (DNA or RNA)   |
| `-nucleic-sidechain`      | Base atoms (DNA or RNA)                       |
| `-nucleic-dna-backbone`   | DNA backbone atoms                            |
| `-nucleic-dna-sidechain`  | DNA base atoms                                |
| `-nucleic-rna-backbone`   | RNA backbone atoms                            |
| `-nucleic-rna-sidechain`  | RNA base atoms                                |

Conceptually:

- Backbone = phosphate + sugar atoms
- Sidechain = nucleobase atoms

### Atom-selection examples

ALA CB atom:

```
[ -rname ALA -aname CB ]
```

C-alpha atoms in chain A:

```
[ -chain A -aname CA ]
```

Protein side-chain atoms in residues 50–100, excluding PRO:

```
( [ -protein-sidechain -rnum 50:100 ] ) and ( not [ -rname PRO ] )
```

## Contact selection

Contact filters match a pair of atom groups (`atom1`, `atom2`) and optional
relationship constraints.

### Selecting atom1 and atom2

Positive forms:

```
-atom1 <atom-filter>   or  -a1 <atom-filter>
-atom2 <atom-filter>   or  -a2 <atom-filter>
```

Negation forms:

```
-atom1-not <atom-filter>   or  -a1!
-atom2-not <atom-filter>   or  -a2!
```

Example:

```
[ -a1 [ -rname ALA -aname CB ] -a2 [ -protein-backbone ] ]
```

### Chain and residue relationships

| Clause           | Meaning                       |
|------------------|-------------------------------|
| `-inter-chain`   | Atoms from different chains   |
| `-intra-chain`   | Atoms from the same chain     |
| `-inter-residue` | Atoms from different residues |

Examples:

```
[ -inter-chain ]
[ -inter-residue ]
```

### Sequence separation (same chain)

```
-min-sep <int>
-max-sep <int>
```

Examples:

```
[ -min-sep 5 ]
[ -max-sep 10 ]
```

### Distance constraint

```
-max-dist <float>
```

Example:

```
[ -max-dist 3.5 ]
```

### Contact-selection examples

ALA CB with protein backbone:

```
[ -a1 [ -rname ALA -aname CB ] -a2 [ -protein-backbone ] ]
```

Inter-chain protein–protein contacts:

```
[ -a1 [ -protein ] -a2 [ -protein ] -inter-chain ]
```

Short side-chain contacts:

```
( [ -a1 [ -protein-sidechain ] -a2 [ -protein-sidechain ] -max-dist 3.5 ] )
```

Complex example:

```
(
  (
    ( [ -a1 [ -aname CA ] -a2 [ -protein-sidechain ] ] )
    and
    ( [ -inter-residue ] )
  )
  or
  ( [ -max-dist 3.0 ] )
)
```

