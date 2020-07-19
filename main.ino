#include <Servo.h>

Servo servo;

byte digit = 0;           // 目前數字 0~9
byte mode = 0;            // 目前模式
byte buttonState = 0;     // 按鈕狀態
byte lastButtonState = 0; // 按鈕最後狀態

unsigned long toggled[10]; // 時間狀態開關
unsigned int step = 0;     // 目前腳本步驟
// 計時間隔 mode 0 ~ 9
unsigned int interval[10] = {1000, 100, 500, 500, 50, 300, 500, 10000, 1000, 1000};

// GWS S35/STD
//byte speeds[21] = {
//    75, 76, 77, 78, 79, 80, 81, 82, 83, 84, // 順時鐘 0 ~ 9
//    90,                                     // 靜止 10
//    91, 92, 93, 94, 95, 96, 97, 98, 99, 100 // 逆時鐘 11 ~ 20
//};

// SG 90
// int speeds[21] = {
//     1400, 1402, 1403, 1405, 1406, 1407, 1408, 1409, 1410, 1411, // 順時鐘 0 ~ 9
//     1450,                                                       // 靜止 10
//     1565, 1566, 1567, 1569, 1570, 1571, 1572, 1573, 1574, 1580  // 逆時鐘 11 ~ 20
// };

// SG 90 只用慢速
int speeds[21] = {
    1407, 1407, 1408, 1408, 1409, 1409, 1410, 1410, 1411, 1411, // 順時鐘 0 ~ 9
    1450,                                                       // 靜止 10
    1565, 1565, 1566, 1566, 1567, 1567, 1569, 1569, 1570, 1570  // 逆時鐘 11 ~ 20
};

byte seven_seg_digits[10][7] = {
    // {1, 1, 1, 1, 1, 1, 0}, // = 0
    {0, 0, 0, 0, 0, 0, 0}, // = null
    {0, 1, 1, 0, 0, 0, 0}, // = 1
    {1, 1, 0, 1, 1, 0, 1}, // = 2
    {1, 1, 1, 1, 0, 0, 1}, // = 3
    {0, 1, 1, 0, 0, 1, 1}, // = 4
    {1, 0, 1, 1, 0, 1, 1}, // = 5
    {1, 0, 1, 1, 1, 1, 1}, // = 6
    // {1, 1, 1, 0, 0, 0, 0}, // = 7
    {1, 1, 1, 0, 1, 1, 1}, // = A
    {1, 1, 1, 1, 1, 1, 1}, // = 8
    {1, 1, 1, 1, 0, 1, 1}  // = 9
};

void setup()
{
    pinMode(2, OUTPUT); // 7 段顯示器 A 腳位
    pinMode(3, OUTPUT); // 7 段顯示器 B 腳位
    pinMode(4, OUTPUT); // 7 段顯示器 C 腳位
    pinMode(5, OUTPUT); // 7 段顯示器 D 腳位
    pinMode(6, OUTPUT); // 7 段顯示器 E 腳位
    pinMode(7, OUTPUT); // 7 段顯示器 F 腳位
    pinMode(8, OUTPUT); // 7 段顯示器 G 腳位
    pinMode(9, INPUT);  // 按鈕腳位
    Serial.begin(9600);
    servo.write(90);
}

void sevenSegWrite(byte digit)
{
    byte pin = 2;
    for (byte seg = 0; seg < 7; ++seg)
    {
        digitalWrite(pin, seven_seg_digits[digit][seg]);
        ++pin;
    }
}

int sin_wave(int degree)
{
    return sin((degree % 360) * PI / 180.0) * 10 + 10;
}
int sawtooth_wave(int degree)
{
    return degree % 11;
}
int triangle_wave(int degree)
{
    return abs(degree % 20 - 10);
}
int curvy_triangle_wave(int degree)
{
    return pow(abs((degree % 200) - 100), 0.5);
}
int square_wave(int degree)
{
    return 5 + (degree % 2) * 5;
}
int trapezoidal_wave(int degree)
{
    int x = degree % 40;
    if (x < 10)
    {
        return 11 + x;
    }
    else if (x < 20)
    {
        return 20;
    }
    else if (x < 30)
    {
        return 20 - x % 10;
    }
    else
    {
        return 10;
    }
}

void loop()
{
    buttonState = digitalRead(9);
    if (buttonState != lastButtonState)
    {
        if (buttonState == HIGH)
        {
            digit = (digit + 1) % 8;
            step = 0;
            switch (digit)
            {
            case 0:
                servo.detach();
                break;
            default:
                servo.attach(10, 500, 2400);
                break;
            }
            Serial.print("Mode: ");
            Serial.println(digit);
        }
        delay(50);
    }
    lastButtonState = buttonState;
    // 設定數字
    sevenSegWrite(digit);
    // 模式 9
    if (digit == 7)
    {
        if ((millis() - toggled[7]) >= interval[7])
        {
            mode = random(1, 6); // 2 ~ 8
            Serial.print("Mode: ");
            Serial.println(mode);
            toggled[7] = millis();
        }
    }
    else
    {
        mode = digit;
    }
    // 燈號模式
    if ((millis() - toggled[mode]) >= interval[mode])
    {
        step = (step + 1) % 3600;
        switch (mode)
        {
        case 1:
            // 正弦波
            // Serial.println(sin_wave(step));
            servo.writeMicroseconds(speeds[sin_wave(step)]);
            toggled[mode] = millis();
            break;
        case 2:
            // 鋸齒波
            // Serial.println(sawtooth_wave(step));
            servo.writeMicroseconds(speeds[sawtooth_wave(step)]);
            toggled[mode] = millis();
            break;
        case 3:
            // 三角波
            // Serial.println(triangle_wave(step));
            servo.writeMicroseconds(speeds[triangle_wave(step)]);
            toggled[mode] = millis();
            break;
        case 4:
            // 彎曲的三角波
            // Serial.println(curvy_triangle_wave(step));
            servo.writeMicroseconds(speeds[curvy_triangle_wave(step)]);
            toggled[mode] = millis();
            break;
        case 5:
            // 方波
            // Serial.println(square_wave(step));
            servo.writeMicroseconds(speeds[square_wave(step)]);
            toggled[mode] = millis();
            break;
        case 6:
            // 梯形波
            // Serial.println(trapezoidal_wave(step));
            servo.writeMicroseconds(speeds[trapezoidal_wave(step)]);
            toggled[mode] = millis();
            break;
        default:
            break;
        }
    }
}