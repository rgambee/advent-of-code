package assembly

type RegisterSet []int
type Instruction struct {
	OpName  string
	A, B, C int
}

type Operator interface {
	Operate(rs RegisterSet, a, b, c int) RegisterSet
	String() string
}

type ADDR_Struct struct{ name string }
type ADDI_Struct struct{ name string }
type MULR_Struct struct{ name string }
type MULI_Struct struct{ name string }
type BANR_Struct struct{ name string }
type BANI_Struct struct{ name string }
type BORR_Struct struct{ name string }
type BORI_Struct struct{ name string }
type SETR_Struct struct{ name string }
type SETI_Struct struct{ name string }
type GTIR_Struct struct{ name string }
type GTRI_Struct struct{ name string }
type GTRR_Struct struct{ name string }
type EQIR_Struct struct{ name string }
type EQRI_Struct struct{ name string }
type EQRR_Struct struct{ name string }

func (s ADDR_Struct) Operate(rs RegisterSet, a, b, c int) RegisterSet {
	rs[c] = rs[a] + rs[b]
	return rs
}
func (s ADDR_Struct) String() string {
	return s.name
}

func (s ADDI_Struct) Operate(rs RegisterSet, a, b, c int) RegisterSet {
	rs[c] = rs[a] + b
	return rs
}
func (s ADDI_Struct) String() string {
	return s.name
}

func (s MULR_Struct) Operate(rs RegisterSet, a, b, c int) RegisterSet {
	rs[c] = rs[a] * rs[b]
	return rs
}
func (s MULR_Struct) String() string {
	return s.name
}

func (s MULI_Struct) Operate(rs RegisterSet, a, b, c int) RegisterSet {
	rs[c] = rs[a] * b
	return rs
}
func (s MULI_Struct) String() string {
	return s.name
}

func (s BANR_Struct) Operate(rs RegisterSet, a, b, c int) RegisterSet {
	rs[c] = rs[a] & rs[b]
	return rs
}
func (s BANR_Struct) String() string {
	return s.name
}

func (s BANI_Struct) Operate(rs RegisterSet, a, b, c int) RegisterSet {
	rs[c] = rs[a] & b
	return rs
}
func (s BANI_Struct) String() string {
	return s.name
}

func (s BORR_Struct) Operate(rs RegisterSet, a, b, c int) RegisterSet {
	rs[c] = rs[a] | rs[b]
	return rs
}
func (s BORR_Struct) String() string {
	return s.name
}

func (s BORI_Struct) Operate(rs RegisterSet, a, b, c int) RegisterSet {
	rs[c] = rs[a] | b
	return rs
}
func (s BORI_Struct) String() string {
	return s.name
}

func (s SETR_Struct) Operate(rs RegisterSet, a, b, c int) RegisterSet {
	rs[c] = rs[a]
	return rs
}
func (s SETR_Struct) String() string {
	return s.name
}

func (s SETI_Struct) Operate(rs RegisterSet, a, b, c int) RegisterSet {
	rs[c] = a
	return rs
}
func (s SETI_Struct) String() string {
	return s.name
}

func (s GTIR_Struct) Operate(rs RegisterSet, a, b, c int) RegisterSet {
	rs[c] = bool2int(a > rs[b])
	return rs
}
func (s GTIR_Struct) String() string {
	return s.name
}

func (s GTRI_Struct) Operate(rs RegisterSet, a, b, c int) RegisterSet {
	rs[c] = bool2int(rs[a] > b)
	return rs
}
func (s GTRI_Struct) String() string {
	return s.name
}

func (s GTRR_Struct) Operate(rs RegisterSet, a, b, c int) RegisterSet {
	rs[c] = bool2int(rs[a] > rs[b])
	return rs
}
func (s GTRR_Struct) String() string {
	return s.name
}

func (s EQIR_Struct) Operate(rs RegisterSet, a, b, c int) RegisterSet {
	rs[c] = bool2int(a == rs[b])
	return rs
}
func (s EQIR_Struct) String() string {
	return s.name
}

func (s EQRI_Struct) Operate(rs RegisterSet, a, b, c int) RegisterSet {
	rs[c] = bool2int(rs[a] == b)
	return rs
}
func (s EQRI_Struct) String() string {
	return s.name
}

func (s EQRR_Struct) Operate(rs RegisterSet, a, b, c int) RegisterSet {
	rs[c] = bool2int(rs[a] == rs[b])
	return rs
}
func (s EQRR_Struct) String() string {
	return s.name
}

func bool2int(b bool) int {
	if b {
		return 1
	}
	return 0
}

var ALL_OPERATORS = map[string]Operator{
	"addr": ADDR_Struct{"addr"},
	"addi": ADDI_Struct{"addi"},
	"mulr": MULR_Struct{"mulr"},
	"muli": MULI_Struct{"muli"},
	"banr": BANR_Struct{"banr"},
	"bani": BANI_Struct{"bani"},
	"borr": BORR_Struct{"borr"},
	"bori": BORI_Struct{"bori"},
	"setr": SETR_Struct{"setr"},
	"seti": SETI_Struct{"seti"},
	"gtir": GTIR_Struct{"gtir"},
	"gtri": GTRI_Struct{"gtri"},
	"gtrr": GTRR_Struct{"gtrr"},
	"eqir": EQIR_Struct{"eqir"},
	"eqri": EQRI_Struct{"eqri"},
	"eqrr": EQRR_Struct{"eqrr"}}
