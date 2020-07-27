#define BLINKER_WIFI                    //官方wifi协议库
#define BLINKER_MIOT_LIGHT              //小爱灯类库
#define BLINKER_PRINT Serial            //串口协议库
#include <Blinker.h>                    //官方库

char auth[] = "a47e610cad3d";   //换成APP获取到的密匙
char ssid[] = "Laughing";          //WiFi账号
char pswd[] = "123321zxc";   //WIFI密码

bool oState = false;
bool fengshanState = false;//风扇是否打开状态

#define BUTTON_1 "btn-abc" //风扇打开关闭按钮
#define BUTTON_3 "btn-wind" //自然风算法打开关闭按钮
#define Slider_1 "ran-active"//打开风扇时长
#define Slider_2 "ran-sleep`"//关闭风扇时长

BlinkerButton Button1(BUTTON_1);
BlinkerButton Button3(BUTTON_3);
BlinkerSlider Slider1(Slider_1);
BlinkerSlider Slider2(Slider_2);

bool wind = false;
int active_time, sleep_time,flag_time;//自定义打开、关闭、标准位时间

void button1_callback(const String & state)//手机打开关闭风扇按键
{
  BLINKER_LOG("get button state: ", state);
  
  if (state == BLINKER_CMD_ON) {
    BLINKER_LOG("Toggle on!");
    Button1.text("已打开");
    Button1.print("on"); //软件显示打开状态
    fengshanState = true; //风扇打开状态
    oState = true;
    digitalWrite(0, LOW); //风扇实际打开
  }
  else if (state == BLINKER_CMD_OFF) {
    BLINKER_LOG("Toggle off!");
    Button1.text("已关闭");
    // Button1.text("Your button name", "describe");
    Button1.print("off");  //软件显示关闭状态
    digitalWrite(0, HIGH); //风扇实际关闭
    fengshanState = false; //风扇关闭状态
    oState = false;
  }
}

void button3_callback(const String & state)//自然风算法控制开关
{
  BLINKER_LOG("get button state: ", state);

  if (state == BLINKER_CMD_ON) {
    Button3.print("on"); //自然风按钮打开
    fengshanState = true; //风扇打开状态
    wind = true; //自然风算法打开状态
  }
  else if (state == BLINKER_CMD_OFF) {
    wind = false; //自然风算法关闭状态
  }
}

void slider1_callback(int32_t value) //设置打开风扇的时长（具体值在APP中控制）
{
  active_time=value; 
  BLINKER_LOG("get slider value: ", value);
}

void slider2_callback(int32_t value) //设置关闭风扇的时长（具体值在APP中控制）
{
  BLINKER_LOG("get slider data: ", value);
  sleep_time=value;
}

void ziranfeng() {        //自然风算法
    if(sleep_time > 1500 || active_time < 200) //将最小风量改为当前设置最大风风量
    {
      sleep_time = 600;
      active_time = flag_time;
    }
    digitalWrite(0, LOW);
    Blinker.delay(active_time);
    digitalWrite(0, HIGH);
    Blinker.delay(sleep_time);
    sleep_time += random(10,40); //每一次随机增加关闭时长 用于降低风量
    active_time -= random(1,30); //每一次随机减少打开时长 用于降低风量
}

void miotPowerState(const String & state)//用户自定义电源类操作的回调函数: 当小爱同学向设备发起控制, 设备端需要有对应控制处理函数
{
  BLINKER_LOG("need set power state: ", state);

  if (state == BLINKER_CMD_ON) { //小爱同学打开命令操作
    Button1.print("on");
    digitalWrite(0, LOW);
    BlinkerMIOT.powerState("on");
    BlinkerMIOT.print();
    Button1.print("on");
    Button3.print("on");
    fengshanState = true;
    oState = true;
  }
  else if (state == BLINKER_CMD_OFF) { //小爱同学关闭命令操作
    digitalWrite(0, HIGH);
    Button1.print("off");
    BlinkerMIOT.powerState("off");
    BlinkerMIOT.print();
    Button3.print("off");
    Button3.print("off");
    fengshanState = false;
    oState = false;
  }
}

void miotQuery(int32_t queryCode)//小爱设备查询接口 用户自定义设备查询的回调函数:当小爱同学向设备发起控制, 设备端需要有对应控制处理函数
{
  BLINKER_LOG("MIOT Query codes: ", queryCode);

  switch (queryCode)
  {
    case BLINKER_CMD_QUERY_ALL_NUMBER :
      BLINKER_LOG("MIOT Query All");//串口输出
      BlinkerMIOT.powerState(oState ? "on" : "off");//反馈电源
      BlinkerMIOT.print();
      break;
    case BLINKER_CMD_QUERY_POWERSTATE_NUMBER :
      BLINKER_LOG("MIOT Query Power State");//串口输出
      BlinkerMIOT.powerState(oState ? "on" : "off");
      BlinkerMIOT.print();
      break;
    default :
      BlinkerMIOT.powerState(oState ? "on" : "off");
      BlinkerMIOT.print();
      break;
  }
}

void miotBright(const String & bright)  //由于小爱没有给出风扇的专有接口 只能以智能灯亮度作为参数值进行控制风扇风量大小
{
    BLINKER_LOG("need set brightness: ", bright);
    active_time = bright.toInt();  //转换为整数值
    active_time = active_time * 25;  //放大打开时长（由于亮度值只有1-100无法正常模拟自然风量 需要放大打开时长）
    flag_time = active_time; //记录设置下来的最大风风量 用于递减到最小风量后恢复到当前设置的值
    wind = true; //自然风算法打开状态
    Button3.print("on");
    Button1.print("on");
    BlinkerMIOT.print();
}

void dataRead(const String & data)
{
  BLINKER_LOG("Blinker readString: ", data);

  Blinker.vibrate();

  uint32_t BlinkerTime = millis();

  Blinker.print("millis = ", BlinkerTime);
}

void setup()
{
  Serial.begin(115200); //初始化串口波特率
  BLINKER_DEBUG.stream(Serial);

  pinMode(0, OUTPUT); //初始化使能pin功能
  digitalWrite(0, HIGH);//初始化使能pin状态
  oState = false;

  Blinker.begin(auth, ssid, pswd); //初始化网路并匹配APP端秘钥
  Blinker.attachData(dataRead);//读取APP端返回校验数据

  BlinkerMIOT.attachPowerState(miotPowerState);//用户自定义电源类操作的回调函数: 当小爱同学向设备发起控制, 设备端需要有对应控制处理函数
  BlinkerMIOT.attachQuery(miotQuery);
  BlinkerMIOT.attachBrightness(miotBright);

  Button1.attach(button1_callback);
  Button3.attach(button3_callback);
  Slider1.attach(slider1_callback);
  Slider2.attach(slider2_callback);
  active_time = 500;
  sleep_time = 600;
  flag_time = 500;
}

void loop()
{
  Blinker.run();
  if(wind == true && fengshanState == true){ //自然风算法触发
    ziranfeng();
  }
}
