#include <Servo.h>

Servo servo;

byte digit = 0;           // 目前數字 0~9
byte buttonState = 0;     // 按鈕狀態
byte lastButtonState = 0; // 按鈕最後狀態
byte speed = 90;          // 速度
byte speedState = 10;     // 速度段數，預設 10 靜止
byte speedDirection = 1;  // 方向
byte speedControl = 0;    // 速度控制
unsigned int skip[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned int freq = 0;
byte mode = 0; // 模式九模式

byte speeds[21] = {
    75, 76, 77, 78, 79, 80, 81, 82, 83, 84,         // 順時鐘
    90,                                             // 靜止
    99, 100, 101, 102, 103, 104, 105, 106, 107, 108 // 逆時鐘
};

byte seven_seg_digits[10][7] = {
    {1, 1, 1, 1, 1, 1, 0}, // = 0
    {0, 1, 1, 0, 0, 0, 0}, // = 1
    {1, 1, 0, 1, 1, 0, 1}, // = 2
    {1, 1, 1, 1, 0, 0, 1}, // = 3
    {0, 1, 1, 0, 0, 1, 1}, // = 4
    {1, 0, 1, 1, 0, 1, 1}, // = 5
    {1, 0, 1, 1, 1, 1, 1}, // = 6
    {1, 1, 1, 0, 0, 0, 0}, // = 7
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
    servo.attach(10); // 伺服機腳位
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
/**海浪
 * 靜止 -> 遞增 -> 遞減 -> 轉向 -> 遞增 -> 遞減
 */
void surfServo(byte start, byte goal)
{
    if (speedState <= start || speedState >= goal)
    {
        speedDirection *= -1;
    }
    speedState = speedState + speedDirection;
}

/**虛線
 * 靜止 -> 正轉
 */
void dottedServo()
{
    if (speedControl == 0)
    {
        speedState = 10;
        speedControl = 1;
    }
    else if (speedControl == 1)
    {
        if (speedDirection == 1)
        {
            speedState = 5;
        }
        else
        {
            speedState = 15;
        }
        speedControl = 0;
    }
}

void surfDottedServo()
{
    if (speedControl == 0)
    {
        speed = 90;
        speedControl = 1;
    }
    else if (speedControl == 1)
    {
        surfServo(0, 20);
        speed = speeds[speedState];
        speedControl = 0;
    }
}

bool delayCount(int count, byte channel)
{
    if (count <= 0)
        count = 1;
    skip[channel] = (skip[channel] + 1) % count;
    return skip[channel] == 0;
}

void loop()
{
    buttonState = digitalRead(9);
    if (buttonState != lastButtonState)
    {
        if (buttonState == HIGH)
        {
            digit = (digit + 1) % 10;
            speed = 90;
            speedState = 10;
            speedControl = 0;
            for (byte channel = 0; channel < 10; channel++)
            {
                skip[channel] = 0;
            }
            if (digit == 0)
            {
                speedDirection = 1;
            }
            else
            {
                speedDirection = random(0, 2) == 0 ? 1 : -1;
            }
            Serial.print("Mode: ");
            Serial.println(digit);
        }
        delay(50);
    }
    lastButtonState = buttonState;
    // 固定頻率處理
    freq = (freq + 1) % 100;
    if (freq == 0)
    {
        if (digit == 1)
        {
            /**模式一
             * 海浪
             */
            if (delayCount(50, 0))
            {
                surfServo(0, 20);
                speed = speeds[speedState];
            }
        }
        else if (digit == 2)
        {
            /**模式二
             * 頻率慢虛線
             */
            if (delayCount(100, 0))
            {
                dottedServo();
                speed = speeds[speedState];
            }
        }
        else if (digit == 3)
        {
            /**模式三
             * 頻率快虛線
             */
            if (delayCount(10, 0))
            {
                dottedServo();
                speed = speeds[speedState];
            }
        }
        else if (digit == 4)
        {
            /**模式四
             * 頻率漸快虛線
             */
            if (delayCount(10, 0))
            {
                surfDottedServo();
            }
        }
        else if (digit == 5)
        {
            /**模式五
             * 跳跳球
             */
            if (delayCount(1000, 1))
            {
                speedState = 10;
                if (speedControl == 0)
                {
                    speedControl = 1;
                    speedDirection = -1;
                }
                else
                {
                    speedControl = 0;
                    speedDirection = 1;
                }
            }
            if (delayCount(10, 0))
            {
                if (speedControl == 0)
                {
                    surfServo(0, 10);
                }
                else
                {
                    surfServo(10, 20);
                }
                speed = speeds[speedState];
            }
        }
        else if (digit == 6)
        {
            /**模式六
             * 隨機時間、速度
             */
            if (delayCount(speedControl, 0))
            {
                speedState = random(0, 30);
                if (speedState >= 20)
                {
                    speedState = 10;
                }
                speed = speeds[speedState];
                speedControl = random(10, 1000);
            }
        }
        else if (digit == 9)
        {
            /**模式九
             * 隨機
             */
            if (delayCount(1000, 1))
            {
                mode = random(0, 5);
                speedDirection = random(0, 2) == 0 ? 1 : -1;
            }
            if (mode == 0)
            {
                if (delayCount(50, 0))
                {
                    surfServo(0, 20);
                    speed = speeds[speedState];
                }
            }
            else if (mode == 1)
            {
                if (delayCount(100, 0))
                {
                    dottedServo();
                    speed = speeds[speedState];
                }
            }
            else if (mode == 2)
            {
                if (delayCount(10, 0))
                {
                    dottedServo();
                    speed = speeds[speedState];
                }
            }
            else if (mode == 3)
            {
                if (delayCount(10, 0))
                {
                    surfDottedServo();
                }
            }
            else if (mode == 4)
            {
                if (delayCount(speedControl, 0))
                {
                    speedState = random(0, 30);
                    if (speedState >= 20)
                    {
                        speedState = 10;
                    }
                    speed = speeds[speedState];
                    speedControl = random(10, 1000);
                }
            }
        }
    }
    // 設定數字
    sevenSegWrite(digit);
    // 設定伺服機
    servo.write(speed);
}