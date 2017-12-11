local int64 = require "int64"
local cjson = require "cjson"

local u64 = int64.unsigned("1111111111")
local i64 = int64.signed("1111111111")

print("compare int64 and uint64: ", u64 == i64)
print("compare signed 1 < signed 2 : ", int64.signed(1) < int64.signed(2))
print("compare signed 1 <= signed 2 : ", int64.signed(1) <= int64.signed(2))
print("compare signed 1 == signed 2 : ", int64.signed(1) == int64.signed(2))
print("compare signed 1 == signed 2 : ", int64.signed(1) == int64.signed(2))
print("compare signed 1 == signed 1 : ", int64.signed(1) == int64.signed(1))
print("compare signed 1 ~= signed 2 : ", int64.signed(1) ~= int64.signed(2))

print("compare signed 1 < unsigned 2 : ", int64.signed(1) < int64.unsigned(2))
print("compare signed 1 <= unsigned 2 : ", int64.signed(1) <= int64.unsigned(2))
print("compare signed 1 == unsigned 2 : ", int64.signed(1) == int64.unsigned(2))
print("compare signed 1 == unsigned 2 : ", int64.signed(1) == int64.unsigned(2))
print("compare signed 1 == unsigned 1 : ", int64.signed(1) == int64.unsigned(1))
print("compare signed 1 ~= unsigned 2 : ", int64.signed(1) ~= int64.unsigned(2))

print("compare signed 1 == number 1 : ", int64.signed(1) == 1)
print("compare unsigned 1 == number 1 : ", int64.unsigned(1) == 1)

print("compare number 1 == signed 1 : ", 1 == int64.signed(1))
print("compare number 1 == unsigned 1 : ", 1 == int64.unsigned(1))


local t = {
  a = int64.signed("999999999999999999"),
  b = int64.signed("999999999999999")
}

local j = cjson.encode(t)
print(j)
local jj = cjson.decode(j)

print("-----")
print(jj.a)
print(jj.b)
print("-----")

assert(t.a == int64.signed(jj.a))
assert(t.b == int64.signed(jj.b))

local json = [[
  {
    "a": 99999999999999,
    "b": 9999999999999999,
    "c": 9999999999999999999,
    "d": -99999999999999,
    "e": -9999999999999999,
    "f": -9199999999999999998,
    "g": 9000000000000000009,
    "h": -9199999999999999099
  }
]]

local dec = cjson.decode(json)

print("=======")
print(dec.a)
print(dec.b)
print(dec.c)
print(dec.d)
print(dec.e)
print(dec.f)
print(dec.g)
print(dec.h)
print("=======")

local a = int64.signed("999999999999999999")
local b = int64.signed(999999999999999)

print(a)
print(b)
print(a+b)
print(a-b)
print(a*b)
print(a/b)


local a1 = int64.unsigned("9999999999999999999")
local b1 = int64.unsigned(99999999999999)

print(a1)
print(b1)
print(a1+b1)
print(a1-b1)
print(a1*b1)
print(a1/b1)


print("concat: " .. a1 .. ":" .. b1)

print("signed 10 + unsigned 10 ", int64.signed(10) + int64.unsigned(10))
print("unsigned 10 + signed 10 ", int64.unsigned(10) + int64.signed(10))

print("signed -10 + unsigned 10 ", int64.signed(-10) + int64.unsigned(10))
print("unsigned 10 + signed -10 ", int64.unsigned(10) + int64.signed(-10))

print("unsigned 10 + signed -20 ", int64.unsigned(10) + int64.signed(-20))
print("signed -10 + unsigned 5 ", int64.signed(-10) + int64.unsigned(5))

print("number 10 + signed 10 ", 10 + int64.signed(10))
print("number 10 + unsigned 10 ", 10 + int64.unsigned(10))

print("number 1 < signed 10 ", 1 < int64.signed(10))
print("signed 10 < number 20", int64.signed(10) < 20)

print("number 1 + signed 1 ", 1 + int64.signed(1))
print("signed 1 + number 1 ", int64.signed(1) + 1)

print("number 1 + unsigned 1 ", 1 + int64.unsigned(1))
print("unsigned 1 + number 1 ", int64.unsigned(1) + 1)

print("unsigned 1 + number 1 + signed 1", int64.unsigned(1) + 1 + int64.signed(1))

print("signed(1):compare(1) ", int64.signed(1):compare(1))
print("signed(1):compare(0) ", int64.signed(1):compare(0))
print("signed(1):compare(2) ", int64.signed(1):compare(2))

print("tonumber(signed(\"9999999999999999\")) ", int64.signed("9999999999999999"):tonumber())

int64.signed("0")