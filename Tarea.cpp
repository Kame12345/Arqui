#include <bitset>
#include <iostream>


std::bitset<32> twosComplement(std::bitset<32> b, int n);
std::bitset<32> binaryAddition(std::bitset<32> a, std::bitset<32> b, int n);
std::bitset<32> arithmeticShiftRight(std::bitset<32> a, std::bitset<32> q,
                                     bool &q_minus_1, int n);
std::bitset<32> boothMultiplication(std::bitset<32> M, std::bitset<32> Q,
                                    int n);

std::bitset<32> decimalToBinary(int num, int n) {
  std::bitset<32> binary;
  bool isNegative = false;
  if (num < 0) {
    isNegative = true;
    num = -num;
  }

  for (int i = 0; i < n; i++) {
    binary[i] = num & 1;
    num >>= 1;
  }

  if (isNegative) {
    binary = twosComplement(binary, n);
  }

  return binary;
}

std::bitset<32> twosComplement(std::bitset<32> b, int n) {
  for (int i = 0; i < n; i++) {
    b[i] = !b[i];
  }
  b = b.to_ulong() + 1;
  return b;
}

std::bitset<32> binaryAddition(std::bitset<32> a, std::bitset<32> b, int n) {
  bool carry = 0;
  std::bitset<32> result;

  for (int i = 0; i < n; i++) {
    result[i] = (a[i] ^ b[i]) ^ carry;
    carry = (a[i] & b[i]) | (a[i] & carry) | (b[i] & carry);
  }
  return result;
}

std::bitset<32> arithmeticShiftRight(std::bitset<32> a, std::bitset<32> q,
                                     bool &q_minus_1, int n) {
  bool temp = q[0];
  q >>= 1;
  q[n - 1] = a[0];
  a >>= 1;
  a[n - 1] = a[n - 1];
  q_minus_1 = temp;

  return a;
}

std::bitset<32> boothMultiplication(std::bitset<32> M, std::bitset<32> Q,
                                    int n) {
  std::bitset<32> A = 0;
  bool Q_minus_1 = 0;
  std::bitset<32> M_neg = twosComplement(M, n);

  for (int i = 0; i < n; i++) {
    if (Q[0] == 1 && Q_minus_1 == 0) {
      A = binaryAddition(A, M_neg, n);
    } else if (Q[0] == 0 && Q_minus_1 == 1) {
      A = binaryAddition(A, M, n);
    }
    A = arithmeticShiftRight(A, Q, Q_minus_1, n);
  }

  for (int i = 0; i < n; i++) {
    A[i + n] = Q[i];
  }

  return A;
}

int main() {
  int n;
  std::cout << "Ingrese número de mips ";
  std::cin >> n;
  int m, q;
  std::cout << "Ingresar multiplicación ";
  std::cin >> m;
  std::cout << "Ingresar multiplicación ";
  std::cin >> q;

  std::bitset<32> M = decimalToBinary(m, n);
  std::bitset<32> Q = decimalToBinary(q, n);

  std::bitset<32> result = boothMultiplication(M, Q, n);

  std::cout << "Resultado en Binario: " << result.to_string().substr(32 - n * 2)
            << std::endl;
  return 0;
}
