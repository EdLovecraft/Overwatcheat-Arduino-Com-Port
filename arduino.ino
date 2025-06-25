#include <Mouse.h>
#include <Wire.h>
#include <SPI.h>

#include <usbhub.h>
USB Usb;
USBHub Hub(&Usb);

#include <hidboot.h>
HIDBoot<USB_HID_PROTOCOL_MOUSE> HidMouse(&Usb);

// --- 状态变量，用于从Parser传递数据到loop ---
// 使用 volatile 关键字，因为这些变量可能在 Usb.Task() 的中断上下文中被修改
volatile int phys_dx = 0, phys_dy = 0, phys_dz = 0;
volatile uint8_t phys_buttons = 0;
uint8_t last_phys_buttons = 0;

// --- 自动瞄准和点击的状态变量 ---
volatile bool isAutoAiming = false;
int aim_dx = 0, aim_dy = 0;

bool isClicking = false;
unsigned long clickStartTime = 0;
unsigned long clickDuration;

// ----- Mouse Report Parser (只更新状态，不执行操作) - 已修正 -----
class MouseRptParser : public MouseReportParser {
protected:
  void OnMouseMove(MOUSEINFO *mi) {
    phys_dx += mi->dX;
    phys_dy += mi->dY;
  };

  void OnMouseScroll(MOUSEINFO *mi) {
    phys_dz += mi->dZ;
  };
  
  // --- 修正部分：使用独立的、正确的按键回调函数 ---
  void OnLeftButtonUp(MOUSEINFO *mi)    { phys_buttons &= ~MOUSE_LEFT; }
  void OnLeftButtonDown(MOUSEINFO *mi)  { phys_buttons |= MOUSE_LEFT; }
  
  void OnRightButtonUp(MOUSEINFO *mi)   { phys_buttons &= ~MOUSE_RIGHT; }
  void OnRightButtonDown(MOUSEINFO *mi) { phys_buttons |= MOUSE_RIGHT; }

  void OnMiddleButtonUp(MOUSEINFO *mi)  { phys_buttons &= ~MOUSE_MIDDLE; }
  void OnMiddleButtonDown(MOUSEINFO *mi){ phys_buttons |= MOUSE_MIDDLE; }
  
  // 注意：Mouse.h 中侧键通常是 MOUSE_XB2 和 MOUSE_XB1
  void OnXButton1Up(MOUSEINFO *mi)      { phys_buttons &= ~MOUSE_XB1; }
  void OnXButton1Down(MOUSEINFO *mi)    { phys_buttons |= MOUSE_XB1; }

  void OnXButton2Up(MOUSEINFO *mi)      { phys_buttons &= ~MOUSE_XB2; }
  void OnXButton2Down(MOUSEINFO *mi)    { phys_buttons |= MOUSE_XB2; }
};

MouseRptParser Prs;

void setup() {
    delay(2000);
    Serial.begin(115200);
    Serial.setTimeout(1);
    
    Mouse.begin();

    if (Usb.Init() == -1) {
        Serial.println("OSC did not start.");
        while (1);
    }
    Serial.println("USB Host Shield initialised");

    HidMouse.SetReportParser(0, &Prs);
    randomSeed(analogRead(0));
}

void loop() {
    // 1. 总是先调用 Usb.Task()
    Usb.Task();

    // 2. 处理串口指令，只更新目标值
    if (Serial.available() > 0) {
        String command = Serial.readStringUntil('\n');
        command.trim();

        if (command.startsWith("M")) {
            int commaIndex = command.indexOf(',');
            if (commaIndex != -1) {
                aim_dx = command.substring(1, commaIndex).toInt();
                aim_dy = command.substring(commaIndex + 1).toInt();
                if (aim_dx != 0 || aim_dy != 0) {
                    isAutoAiming = true;
                }
            }
        } else if (command.startsWith("C")) {
            if (!isClicking) {
                // 对于串口触发的点击，我们直接操作，因为它不涉及USB Host输入
                Mouse.press(MOUSE_LEFT);
                clickStartTime = millis();
                clickDuration = random(40, 80);
                isClicking = true;
            }
        }
    }

    // 3. 统一处理所有鼠标移动
    int final_dx = 0;
    int final_dy = 0;
    int final_dz = phys_dz;

    if (isAutoAiming) {
        int moveX = constrain(aim_dx, -127, 127);
        int moveY = constrain(aim_dy, -127, 127);
        final_dx = moveX;
        final_dy = moveY;
        
        aim_dx -= moveX;
        aim_dy -= moveY;
        
        if (aim_dx == 0 && aim_dy == 0) {
            isAutoAiming = false;
        }
    } else {
        final_dx = phys_dx;
        final_dy = phys_dy;
    }

    if (final_dx != 0 || final_dy != 0 || final_dz != 0) {
        Mouse.move(final_dx, final_dy, final_dz);
    }
    phys_dx = 0;
    phys_dy = 0;
    phys_dz = 0;

    // 4. 统一处理所有来自物理鼠标的按键
    uint8_t current_buttons = phys_buttons; // 将 volatile 变量复制到局部变量中，确保在比较期间值不变
    if (current_buttons != last_phys_buttons) {
        // 检查哪些按键被新按下了
        if ((current_buttons & MOUSE_LEFT) && !(last_phys_buttons & MOUSE_LEFT)) Mouse.press(MOUSE_LEFT);
        if ((current_buttons & MOUSE_RIGHT) && !(last_phys_buttons & MOUSE_RIGHT)) Mouse.press(MOUSE_RIGHT);
        if ((current_buttons & MOUSE_MIDDLE) && !(last_phys_buttons & MOUSE_MIDDLE)) Mouse.press(MOUSE_MIDDLE);
        if ((current_buttons & MOUSE_XB2) && !(last_phys_buttons & MOUSE_XB2)) Mouse.press(MOUSE_XB2);
        if ((current_buttons & MOUSE_XB1) && !(last_phys_buttons & MOUSE_XB1)) Mouse.press(MOUSE_XB1);

        // 检查哪些按键被释放了
        if (!(current_buttons & MOUSE_LEFT) && (last_phys_buttons & MOUSE_LEFT)) Mouse.release(MOUSE_LEFT);
        if (!(current_buttons & MOUSE_RIGHT) && (last_phys_buttons & MOUSE_RIGHT)) Mouse.release(MOUSE_RIGHT);
        if (!(current_buttons & MOUSE_MIDDLE) && (last_phys_buttons & MOUSE_MIDDLE)) Mouse.release(MOUSE_MIDDLE);
        if (!(current_buttons & MOUSE_XB2) && (last_phys_buttons & MOUSE_XB2)) Mouse.release(MOUSE_XB2);
        if (!(current_buttons & MOUSE_XB1) && (last_phys_buttons & MOUSE_XB1)) Mouse.release(MOUSE_XB1);
        
        last_phys_buttons = current_buttons;
    }
    
    // 5. 处理串口触发的点击的释放逻辑
    if (isClicking && millis() - clickStartTime >= clickDuration) {
        Mouse.release(MOUSE_LEFT);
        isClicking = false;
    }

    // 6. 在loop末尾加入微小延迟以增加稳定性
    delay(1);
}
