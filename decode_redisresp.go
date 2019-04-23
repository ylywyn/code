package main

import (
	"bufio"
	"bytes"
	"io"
	"strconv"
)

const (
	TypeString    = '+'
	TypeError     = '-'
	TypeInt       = ':'
	TypeBulkBytes = '$'
	TypeArray     = '*'
)

const (
	MaxBulkBytesLen = 1024 * 1024 * 512
	MaxArrayLen     = 1024 * 1024
)

func DecodeFromBytes(p []byte) (*Resp, error) {
	return NewDecoder(bytes.NewReader(p)).Decode()
}

type Decoder struct {
	br  *bufio.Reader
	Err error
}

type Resp struct {
	Type  byte
	Value []byte
	Array []*Resp
}

// NewDecoder
func NewDecoder(r io.Reader) *Decoder {
	d := &Decoder{br: bufio.NewReaderSize(r, 8192)}
	return d
}

// Decode
func (d *Decoder) Decode() (*Resp, error) {
	if d.Err != nil {
		return nil, d.Err
	}
	r, err := d.decodeResp()
	if err != nil {
		d.Err = err
	}
	return r, d.Err
}

// decodeResp 根据返回类型调用不同解析实现
func (d *Decoder) decodeResp() (*Resp, error) {
	b, err := d.br.ReadByte()
	if err != nil {
		return nil, err
	}
	r := &Resp{}
	r.Type = byte(b)
	switch r.Type {
	default:
		return nil, err
	case TypeString, TypeError, TypeInt:
		r.Value, err = d.decodeTextBytes()
	case TypeBulkBytes:
		r.Value, err = d.decodeBulkBytes()
	case TypeArray:
		r.Array, err = d.decodeArray()
	}
	return r, err
}

// decodeTextBytes decode文本
func (d *Decoder) decodeTextBytes() ([]byte, error) {
	b, err := d.br.ReadBytes('\n')
	if err != nil {
		return nil, err
	}
	if n := len(b) - 2; n < 0 || b[n] != '\r' {
		return nil, err
	} else {
		return b[:n], nil
	}
}

// decodeInt decode int
func (d *Decoder) decodeInt() (int64, error) {
	b, err := d.br.ReadSlice('\n')
	if err != nil {
		return 0, err
	}
	if n := len(b) - 2; n < 0 || b[n] != '\r' {
		return 0, err
	} else {
		return strconv.ParseInt(string(b[:n]), 10, 0)
	}
}

// decodeBulkBytes decode 批量回复
func (d *Decoder) decodeBulkBytes() ([]byte, error) {
	n, err := d.decodeInt()
	if err != nil {
		return nil, err
	}
	switch {
	case n < -1:
		return nil, err
	case n > MaxBulkBytesLen:
		return nil, err
	case n == -1:
		return nil, nil
	}

	b := make([]byte, int(n)+2)
	_, err = d.br.Read(b)
	if err != nil {
		return nil, err
	}
	if b[n] != '\r' || b[n+1] != '\n' {
		return nil, err
	}
	return b[:n], nil
}

// decodeArray decode 多条批量回复
func (d *Decoder) decodeArray() ([]*Resp, error) {
	n, err := d.decodeInt()
	if err != nil {
		return nil, err
	}
	switch {
	case n < -1:
		return nil, err
	case n > MaxArrayLen:
		return nil, err
	case n == -1:
		return nil, nil
	}
	array := make([]*Resp, n)
	for i := range array {
		r, err := d.decodeResp()
		if err != nil {
			return nil, err
		}
		array[i] = r
	}
	return array, nil
}
