// RUN: %goop-tok < %s | FileCheck %s

package main

func main() {
}

// CHECK: Keyword(kind: package)
// CHECK-NEXT: Identifier(ident: main)

// CHECK: Keyword(kind: func)
// CHECK-NEXT: Identifier(ident: main)
// CHECK-NEXT: Punctuation(kind: ()
// CHECK-NEXT: Punctuation(kind: ))
// CHECK-NEXT: Punctuation(kind: {)
// CHECK-NEXT: Punctuation(kind: })
