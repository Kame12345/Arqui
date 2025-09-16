#include <iostream>
#include <bitset>
#include <cmath>
#include <iomanip>
#include <vector>
#include <stdint.h>

using namespace std;

class FloatMultiplier {
private:

    static const int BIAS = 127;
    static const int MANTISSA_BITS = 23;
    static const int EXPONENT_BITS = 8;
    

    struct FloatComponents {
        uint32_t sign;
        uint32_t exponent;
        uint32_t mantissa;
        bool isZero;
        bool isInfinity;
        bool isNaN;
    };
    
public:

    FloatComponents extractComponents(float value) {
        FloatComponents comp;
        uint32_t bits = *reinterpret_cast<uint32_t*>(&value);
        
        comp.sign = (bits >> 31) & 0x1;
        comp.exponent = (bits >> 23) & 0xFF;
        comp.mantissa = bits & 0x7FFFFF;
        
        comp.isZero = (comp.exponent == 0 && comp.mantissa == 0);
        comp.isInfinity = (comp.exponent == 0xFF && comp.mantissa == 0);
        comp.isNaN = (comp.exponent == 0xFF && comp.mantissa != 0);
        
        return comp;
    }
    

    void showBitPattern(float value, const string& label) {
        uint32_t bits = *reinterpret_cast<uint32_t*>(&value);
        FloatComponents comp = extractComponents(value);
        
        cout << label << ": " << value << endl;
        cout << "Bits: " << bitset<32>(bits) << endl;
        cout << "Signo: " << comp.sign;
        cout << " | Exponente: " << bitset<8>(comp.exponent);
        cout << " | Mantisa: " << bitset<23>(comp.mantissa) << endl;
        cout << "Exponente decimal: " << comp.exponent;
        if (comp.exponent != 0 && comp.exponent != 0xFF) {
            cout << " (sin bias: " << (comp.exponent - BIAS) << ")";
        }
        cout << endl << endl;
    }
    

    float multiply(float x, float y) {
        cout << "=== ALGORITMO DE MULTIPLICACIÓN PUNTO FLOTANTE ===" << endl;
        

        FloatComponents compX = extractComponents(x);
        FloatComponents compY = extractComponents(y);
        
        cout << "Paso 1: Verificar si X = 0 o Y = 0" << endl;
        

        if (compX.isZero || compY.isZero) {
            cout << "Uno de los valores es 0, resultado = 0" << endl;
            return 0.0f;
        }
        
        cout << "Ningún valor es 0, continuar..." << endl << endl;
        

        uint32_t resultSign = compX.sign ^ compY.sign;
        cout << "Paso 2: Calcular signo resultado" << endl;
        cout << "Signo X: " << compX.sign << ", Signo Y: " << compY.sign;
        cout << " -> Signo resultado: " << resultSign << endl << endl;
        

        cout << "Paso 3: Sumar exponentes" << endl;
        int expX = compX.exponent;
        int expY = compY.exponent;
        

        if (compX.exponent == 0) expX = 1;
        if (compY.exponent == 0) expY = 1;
        
        int sumExp = expX + expY - BIAS;
        cout << "Exp X: " << expX << ", Exp Y: " << expY;
        cout << " -> Suma: " << (expX + expY) << " - bias(" << BIAS << ") = " << sumExp << endl;
        

        cout << "Paso 4: Verificar overflow/underflow de exponente" << endl;
        if (sumExp >= 255) {
            cout << "Overflow de exponente detectado! Resultado = infinito" << endl;
            uint32_t resultBits = (resultSign << 31) | (0xFF << 23);
            return *reinterpret_cast<float*>(&resultBits);
        }
        
        if (sumExp <= 0) {
            cout << "Underflow de exponente detectado! Resultado = 0" << endl;
            return (resultSign == 1) ? -0.0f : 0.0f;
        }
        
        cout << "Exponente dentro del rango válido" << endl << endl;
        

        cout << "Paso 5: Multiplicar significandos" << endl;
        

        uint64_t mantX = compX.mantissa;
        uint64_t mantY = compY.mantissa;
        
        if (compX.exponent != 0) mantX |= (1ULL << MANTISSA_BITS);
        if (compY.exponent != 0) mantY |= (1ULL << MANTISSA_BITS);
        
        cout << "Mantisa X (con bit implícito): " << bitset<24>(mantX) << endl;
        cout << "Mantisa Y (con bit implícito): " << bitset<24>(mantY) << endl;
        
        uint64_t productMant = mantX * mantY;
        cout << "Producto mantisas: " << bitset<48>(productMant) << endl;
        

        cout << "Paso 6: Normalización" << endl;
        
        int shift = 0;

        if (productMant & (1ULL << 47)) {
            productMant >>= 1;
            sumExp++;
            shift = 1;
            cout << "Desplazamiento derecha de 1 bit, exponente incrementado a: " << sumExp << endl;
        }

        else if (!(productMant & (1ULL << 46))) {
            while (!(productMant & (1ULL << 46)) && sumExp > 1) {
                productMant <<= 1;
                sumExp--;
                shift--;
            }
            cout << "Desplazamiento izquierda de " << (-shift) << " bits, exponente: " << sumExp << endl;
        }
        
        cout << "Producto normalizado: " << bitset<48>(productMant) << endl;
        

        if (sumExp >= 255) {
            cout << "Overflow después de normalización! Resultado = infinito" << endl;
            uint32_t resultBits = (resultSign << 31) | (0xFF << 23);
            return *reinterpret_cast<float*>(&resultBits);
        }
        
        if (sumExp <= 0) {
            cout << "Underflow después de normalización! Resultado = 0" << endl;
            return (resultSign == 1) ? -0.0f : 0.0f;
        }
        

        cout << "Paso 7: Redondeo" << endl;
        

        uint32_t finalMant = (productMant >> 23) & 0x7FFFFF;
        

        bool roundBit = (productMant >> 22) & 1;

        bool stickyBits = (productMant & 0x3FFFFF) != 0;
        
        cout << "Mantisa antes de redondeo: " << bitset<23>(finalMant) << endl;
        cout << "Round bit: " << roundBit << ", Sticky bits: " << stickyBits << endl;
        

        if (roundBit && (stickyBits || (finalMant & 1))) {
            finalMant++;
            cout << "Redondeo aplicado" << endl;
            
 
            if (finalMant > 0x7FFFFF) {
                finalMant = 0;
                sumExp++;
                cout << "Overflow en mantisa, exponente incrementado a: " << sumExp << endl;
                
                if (sumExp >= 255) {
                    cout << "Overflow final! Resultado = infinito" << endl;
                    uint32_t resultBits = (resultSign << 31) | (0xFF << 23);
                    return *reinterpret_cast<float*>(&resultBits);
                }
            }
        }
        
        cout << "Mantisa final: " << bitset<23>(finalMant) << endl;
        cout << "Exponente final: " << sumExp << endl << endl;
        

        cout << "Paso 8: Ensamblar resultado final" << endl;
        uint32_t resultBits = (resultSign << 31) | ((sumExp & 0xFF) << 23) | (finalMant & 0x7FFFFF);
        float result = *reinterpret_cast<float*>(&resultBits);
        
        cout << "Bits resultado: " << bitset<32>(resultBits) << endl;
        cout << "Valor resultado: " << result << endl;
        
        return result;
    }
};

int main() {
    FloatMultiplier multiplier;
    float x, y;
    
    cout << "=== EMULADOR DE MULTIPLICACIÓN PUNTO FLOTANTE IEEE 754 ===" << endl;
    cout << "Ingrese el primer número (float): ";
    cin >> x;
    cout << "Ingrese el segundo número (float): ";
    cin >> y;
    
    cout << endl << "=== ANÁLISIS DE ENTRADAS ===" << endl;
    multiplier.showBitPattern(x, "Número X");
    multiplier.showBitPattern(y, "Número Y");
    

    cout << "=== MULTIPLICACIÓN EMULADA ===" << endl;
    float emulatedResult = multiplier.multiply(x, y);
    
    float nativeResult = x * y;
    
    cout << endl << "=== COMPARACIÓN DE RESULTADOS ===" << endl;
    multiplier.showBitPattern(emulatedResult, "Resultado Emulado");
    multiplier.showBitPattern(nativeResult, "Resultado Nativo (C++)");
    
    cout << "=== VERIFICACIÓN DE DISCREPANCIAS ===" << endl;
    uint32_t emulatedBits = *reinterpret_cast<uint32_t*>(&emulatedResult);
    uint32_t nativeBits = *reinterpret_cast<uint32_t*>(&nativeResult);
    
    if (emulatedBits == nativeBits) {
        cout << "✓ Los resultados son IDÉNTICOS a nivel de bits" << endl;
    } else {
        cout << "✗ DISCREPANCIA detectada:" << endl;
        cout << "Diferencia en bits: " << bitset<32>(emulatedBits ^ nativeBits) << endl;
        cout << "Diferencia numérica: " << (emulatedResult - nativeResult) << endl;
    }
    
    cout << endl << "=== CASOS DE PRUEBA ADICIONALES ===" << endl;
    
    vector<pair<float, float> > testCases = {
        make_pair(0.0f, 5.0f),
        make_pair(1.0f, 1.0f),
        make_pair(2.0f, 3.0f),
        make_pair(0.1f, 0.2f),
        make_pair(-2.0f, 3.0f),
        make_pair(1.5f, 2.5f),
        make_pair(1e-20f, 1e-20f),
        make_pair(1e20f, 1e20f)
    };
    
    for (size_t i = 0; i < testCases.size(); i++) {
        float a = testCases[i].first;
        float b = testCases[i].second;
        float emulated = multiplier.multiply(a, b);
        float native = a * b;
        
        cout << a << " × " << b << " = ";
        cout << "Emulado: " << emulated << ", Nativo: " << native;
        
        uint32_t eBits = *reinterpret_cast<uint32_t*>(&emulated);
        uint32_t nBits = *reinterpret_cast<uint32_t*>(&native);
        
        if (eBits == nBits) {
            cout << " ✓" << endl;
        } else {
            cout << " ✗ (diff: " << (emulated - native) << ")" << endl;
        }
    }
    
    return 0;
}
