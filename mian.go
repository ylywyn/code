package main

import (
	"fmt"
	"strconv"
)

func main() {
	fmt.Println("hello")

	if resp, err := DecodeFromBytes([]byte("+OK\r\n")); err != nil {
		fmt.Println(err.Error())
	} else {
		printRedisResp(resp) // "OK"
	}

	if resp, err := DecodeFromBytes([]byte(":1000\r\n")); err != nil {
		fmt.Println(err.Error())
	} else {
		printRedisResp(resp) // 1000
	}
}

func printRedisResp(resp *Resp) {
	switch resp.Type {
	case TypeString:
		fmt.Println(string(resp.Value))
	case TypeInt:
		i, _ := strconv.Atoi(string(resp.Value))
		fmt.Println(i)
	}
}
