// RUN: %goop-tok < %s | FileCheck %s

package main

import "fmt"

func main() {
    x := 1
    fmt.Printf("%v", x)

    go func() { x++ }()
}

// CHECK: Keyword(kind: package)
// CHECK-NEXT: Identifier(ident: main)
// CHECK-NEXT: Punctuation(kind: ;)

// CHECK: Keyword(kind: import)
// CHECK-NEXT: StringLiteral(literal: "fmt")
// CHECK-NEXT: Punctuation(kind: ;)

// CHECK: Keyword(kind: func)
// CHECK-NEXT: Identifier(ident: main)
// CHECK-NEXT: Punctuation(kind: ()
// CHECK-NEXT: Punctuation(kind: ))
// CHECK-NEXT: Punctuation(kind: {)
// CHECK-NEXT: Identifier(ident: x)
// CHECK-NEXT: Punctuation(kind: :=)
// CHECK-NEXT: IntLiteral(lit: 1, value: 1, radix: 10)
// CHECK-NEXT: Punctuation(kind: ;)

// CHECK-NEXT: Identifier(ident: fmt)
// CHECK-NEXT: Punctuation(kind: .)
// CHECK-NEXT: Identifier(ident: Printf)
// CHECK-NEXT: Punctuation(kind: ()
// CHECK-NEXT: StringLiteral(literal: "%v")
// CHECK-NEXT: Punctuation(kind: ,)
// CHECK-NEXT: Identifier(ident: x)
// CHECK-NEXT: Punctuation(kind: ))
// CHECK-NEXT: Punctuation(kind: ;)

// CHECK-NEXT: Keyword(kind: go)
// CHECK-NEXT: Keyword(kind: func)
// CHECK-NEXT: Punctuation(kind: ()
// CHECK-NEXT: Punctuation(kind: ))
// CHECK-NEXT: Punctuation(kind: {)
// CHECK-NEXT: Identifier(ident: x)
// CHECK-NEXT: Punctuation(kind: ++)
// CHECK-NEXT: Punctuation(kind: })
// CHECK-NEXT: Punctuation(kind: ()
// CHECK-NEXT: Punctuation(kind: ))
// CHECK-NEXT: Punctuation(kind: ;)

// CHECK-NEXT: Punctuation(kind: })
