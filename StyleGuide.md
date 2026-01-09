# Violet Chess Engine Programming Style Guide

This document outlines the programming style and conventions for the Violet chess engine. The goal is to maintain consistency across the codebase, following the style established in `Bitboard.cpp`.

## 1. General Formatting

### 1.1 Indentation
*   Use **3 spaces** for indentation.
*   Do not use tabs.

### 1.2 Braces
*   Use the **Allman style** (also known as BSD style).
*   Opening and closing braces must be on their own lines.
*   Braces should be aligned with the control statement.

```cpp
if ( condition )
{
   // Code
}
```

### 1.3 Spacing

*   **Parentheses:** Add a space after the opening parenthesis and before the closing parenthesis.
    *   `if ( condition )`
    *   `assert( argsBoard > 0 );`
    *   `for ( int i = 0; i < 10; i++ )`
*   **Operators:** Surround binary operators with spaces.
    *   `iSquare = rowIndex + ( colIndex << 3 );`
*   **Brackets:** Add at least one space inside array brackets. More spaces should be added if required for vertical alignment (see Section 1.5).
    *   `vBoard[ iSquare ]`
*   **Blank Lines:**
    *   Use blank lines freely to separate logical blocks of code.
    *   Often, a blank line is placed immediately after an opening brace `{` and before a closing brace `}` if the block is non-trivial.

### 1.4 Line Length
*   Keep lines reasonably short. Break long lines to improve readability.

### 1.5 Vertical Alignment

*   **Columnar Layout:** In blocks of related assignments or repetitive logic, align operators (`=`), variable names, array indices, and values vertically.
*   **Visual Patterns:** This maximizes the visibility of patterns, making typos and copy-paste errors stand out visually.
*   **Padding:** Insert extra spaces inside brackets or before operators to ensure columns line up perfectly.

```cpp
// Align the brackets, the index variable, the equals sign, and the value.
sGeneralMove->bbRMove[   square ] = 0;
sGeneralMove->bbRMoveN[  square ] = 0;
sGeneralMove->bbRMoveW[  square ] = 0;
sGeneralMove->bbRMoveE[  square ] = 0;
sGeneralMove->bbRMoveS[  square ] = 0;
sGeneralMove->bbQMove[   square ] = 0;
sGeneralMove->bbQMoveNE[ square ] = 0;
sGeneralMove->bbQMoveNW[ square ] = 0;
sGeneralMove->bbQMoveSE[ square ] = 0;
sGeneralMove->bbQMoveSW[ square ] = 0;
```

## 2. Naming Conventions (Hungarian Notation)

Variables should use Hungarian notation to indicate their type.

### 2.1 Prefixes

| Type | Prefix | Example |
| :--- | :--- | :--- |
| **Integer** | `i` | `iSquare`, `iBestMove` |
| **Signed Integer / Index** | `si` | `siPosition`, `siCount` |
| **BitBoard** (`unsigned long long`) | `bb` | `bbWhitePawn`, `bbOccupied` |
| **Pointer / Array** | `v` | `vPosition`, `vBoard` |
| **Struct (Instance)** | `s` | `sDummyMoves` |
| **Struct (Argument/Pointer)** | `args` | `argsBoard`, `argsGeneralMoves` |
| **Member Variable** | `m` | `mBoard` |
| **Char** | `c` | `cPiece` |
| **Define / Constant** | `d` | `dWhitePawn`, `dYes`, `dNo` |
| **Define Score** | `ds` | `dsCapture`, `dsPawnTwo` |

### 2.2 Variable Names
*   Use PascalCase after the prefix.
*   Example: `iMoveHistory`, `bbWhiteRook`.

### 2.3 Function Names
*   Use PascalCase.
*   Example: `CreateBoard`, `PrintBitBoard`.

## 3. Functions

### 3.1 Definition Format
*   Return type on the same line as the function name.
*   **Arguments:** Place each argument on its own line.
*   Align the types of the arguments if possible, or simply indent them to align with the opening parenthesis.

```cpp
void CreateBoard( struct Board * argsBoard,
                  struct GeneralMove * argsGeneralMoves )
{
   // ...
}
```

### 3.2 Separators
*   Use a comment block with dashes to separate functions.

```cpp
//
//---------------------------------------------------------------------
//
void NextFunction()
{
   // ...
}
```

## 4. Comments

*   Use `//` for single-line comments.
*   Place comments *above* the code they explain.
*   Comments should be complete sentences when possible.
*   Indent comments to match the code level.

```cpp
   // Set the initial best move to something higher than any number of moves.
   argsBoard->iBestMove = 128;
```

## 5. Structs and Types

*   Use `struct` keyword explicitly in function arguments when passing structs (C-style influence).
*   Example: `struct Board * argsBoard`.

## 6. Example Snippet

```cpp
//
//---------------------------------------------------------------------
//
int Find( BitBoard bbBoard, 
          int * vPosition, 
          struct GeneralMove * argsGeneralMoves ) 
{
   int siCount = 0;
   unsigned long index;

   // This function finds the non-zero bits.
   while ( _BitScanForward64( &index, bbBoard ) ) 
   {
   
      vPosition[ siCount++ ] = index;
      bbBoard &= bbBoard - 1; // Clear the LSB
      
   }
   
   return siCount;
}
```

## 7. Assertions

To ensure reliability and catch logic errors early, `assert` statements should be used wherever practical throughout the codebase to verify assumptions and data integrity.

### 7.1 Usage Guidelines
* **Pointer Validation:** Always check that pointers passed as arguments are not `NULL` or `0` before dereferencing them.
* **Bounds Checking:** Verify that indices (such as board squares, array lookups) are within valid ranges (e.g., $0$ to $63$ for squares).
* **Logic Verification:** Assert that impossible states (like negative move counts or invalid piece types) do not occur.

### 7.2 Formatting
* **Spacing:** Follow the standard spacing rules. Add a space after the opening parenthesis and before the closing parenthesis.
    * `assert( argsBoard != NULL );`
* **Placement:** Generally place assertions at the very top of the function to validate inputs immediately.

```cpp
//
//---------------------------------------------------------------------
//
void UpdatePiece( struct Board * argsBoard, 
                  int iSquare, 
                  int iPieceType )
{
   // Validate the board pointer.
   assert( argsBoard != NULL );

   // Ensure the square is within board bounds (0-63).
   assert( iSquare >= 0 );
   assert( iSquare < 64 );
   
   // ... code ...
}
```