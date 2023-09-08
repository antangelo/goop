// RUN: %goop-ast < %s | FileCheck %s

// CHECK: PackageClause
// CHECK-SAME: ident: main
package main

// CHECK: ImportSpec [
// CHECK-SAME: StringLiteral(literal: "math")
import "math"

const ( aconstant = 1 + 2 / 3 )
var name = float64(aconstant) + 2 * 3 + -4 * math.Sin(10)
var x int

const (
    a = 1
    b
    c
)

//type x = map[uint64][]*chan uint64

/*
type s = struct {
    a, b, c int64
}
*/
