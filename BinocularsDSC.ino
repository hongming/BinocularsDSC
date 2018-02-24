// 引入各个库函数
#include "Encoder.h"
#include "DS1302.h"
#include "math.h"

// 定义两个编码器
Encoder Encoder_Azimuth(2, 3);
Encoder Encoder_Altitude(4, 5);
//编码器数值，前一次
long Encoder_Azimuth_oldPosition = -999;
long Encoder_Altitude_oldPosition = -999;
//编码器数值
long Encoder_Azimuth_Position;
long Encoder_Altitude_Position;
//编码器对应的方位弧度
float Encoder_Azimuth_Radian;
float Encoder_Altitude_Radian;
// 定义变量：观察者的经纬度，地理经度是从子午圈向西测量的，而不是向东，注意注意注意
double GPS_Longitude;
double GPS_Latitude;
// 定义变量：望远镜指向的赤经和赤纬，按小数和整数拆分显示；
float Astro_HUD_RA;
int Mod_RA_HH, Mod_RA_MM, Mod_RA_SS;
float Astro_HUD_DEC;
int Mod_DEC_DD, Mod_DEC_MM, Mod_DEC_SS;
// 儒略日和简化儒略日；
double JD, MJD;
// 本地时间
float Year, Month, Day, Hour, Minute, Second;
// 格林尼治恒星时
double Siderial_Time;
// 本地恒星时 LST
double Siderial_Time_Local;
// 读取串口数据，以字符串传递给Stellarium；
char inChar;
String Stellarium = "";
// RTC时间相关
namespace
{
const int kCePin = 5; // Chip Enable
const int kIoPin = 6; // Input/Output
const int kSclkPin = 7; // Serial Clock
DS1302 rtc(kCePin, kIoPin, kSclkPin);
}

void setup()
{
  Serial.begin(9600);
  Serial.println("Binoculars Digital Setting Circle");
  // 输入观察者所在纬度、经度
  GPS_Latitude = 31.0456;
  //Observer's longitude (λo) here is measured positively westward from the prime meridian; this is contrary to current IAU standards.https://en.wikipedia.org/wiki/Celestial_coordinate_system#Converting_coordinates
  GPS_Longitude = -121.3997;
  // 启动RTC、设置时间
  rtc.writeProtect(false);
  rtc.halt(false);
  //以下仅用于重置时钟的时间，平时需注释掉
  Time t(2018, 02, 26, 22, 55, 35, Time::kSunday);
  rtc.time(t);	// Set the time and date on the chip.
}

void loop()
{
  // 以下获得实时时间
  //  Time t = rtc.time();
  //  // 时间按格式分拆
  //  	Year = t.yr;
  //  	Month = t.mon;
  //  	Day = t.date;
  //  	Hour = t.hr;
  //Hour = Hour - 8;	//换算到UTC时间，这里可能会有问题
  //  	Minute = t.min;
  //  	Second = t.sec;
  Year = 2018;
  Month = 2;
  Day = 19;
  Hour = 20;
  Hour = Hour - 8;  //换算到UTC时间，这里可能会有问题
  Minute = 20;
  Second = 10;

  // 儒略日，计算采用Navy.mil的计算试试看
  JD = 367 * Year - int((7 * (Year + int((Month + 9) / 12))) / 4) + int((275 * Month) / 9) + Day + 1721013.5 + Hour / 24 + Minute / 1440 + Second / 86400 - 0.5 * ((((100 * Year + Month - 190002.5) > 0) - ((100 * Year + Month - 190002.5) < 0))) + 0.5;
  // 简化儒略日，计算
  MJD = JD - 2400000.5;
  Siderial_Time = 18.697374558 + 24.06570982441908 * (JD - 2451545.0);
  // 本地恒星时(-1!!!!!!)
  Siderial_Time_Local = fmod(Siderial_Time + (-1)*GPS_Longitude / 15,24);
  // if (Siderial_Time_Local < -24)
  // {
  //   Siderial_Time_Local = 24 + abs(Siderial_Time_Local) + Siderial_Time_Local;
  // }
  // if (Siderial_Time_Local < 0 && Siderial_Time_Local > -24)
  // {
  //   Siderial_Time_Local = 24.0 - abs(Siderial_Time_Local);
  // }
  // if (Siderial_Time_Local > 24.0)
  // {
  //   Siderial_Time_Local = (Siderial_Time_Local - floor(Siderial_Time_Local)) + int(Siderial_Time_Local) % 24;
  // }
  // Serial.print(JD);
  // Serial.print("\t");
  // Serial.print(MJD);
  // Serial.print("\t");
   Serial.println(Siderial_Time_Local);

  //编码器部分
  //Encoder_Azimuth_Position = Encoder_Azimuth.read();
  //Encoder_Altitude_Position = Encoder_Altitude.read();
Encoder_Altitude_Position=0;
//   //Encoder_Altitude_Position=787;
// Encoder_Altitude_Position=1057;//80度

  //往哪个方向？！！！
  for(Encoder_Azimuth_Position = 0;Encoder_Azimuth_Position<4800;Encoder_Azimuth_Position++)
  {
  // 任一编码器数值变动时，打印出当前两个编码器的数值；
  if ((Encoder_Azimuth_Position != Encoder_Azimuth_oldPosition) || (Encoder_Azimuth_Position != Encoder_Altitude_oldPosition))
  {
    Encoder_Azimuth_oldPosition = Encoder_Azimuth_Position;
    Encoder_Altitude_oldPosition = Encoder_Altitude_Position;
        Serial.print("Azimuth Value: \t");
        Serial.print(Encoder_Azimuth_Position);
        // Serial.print("\t Altitude Value: \t");
        // Serial.print(Encoder_Altitude_Position);
  }
  
  Encoder_Azimuth_Radian = ((abs(Encoder_Azimuth_Position) % 4800) * 360.0 / 4800) * 2 * PI / 360;
  Encoder_Altitude_Radian = ((abs(Encoder_Altitude_Position) % 4800) * 360.0 / 4800) * 2 * PI / 360;
  // Serial.print("\t");
  // Serial.println(Encoder_Azimuth_Radian);
  //Serial.print("\t");
  //Serial.println(Encoder_Altitude_Radian);

  // 计算赤经
  Astro_HUD_RA = Siderial_Time_Local - atan(sin(Encoder_Azimuth_Radian) / (cos(Encoder_Azimuth_Radian) * sin(GPS_Latitude * (2 * PI / 360)) - tan(Encoder_Altitude_Radian) * cos(GPS_Latitude * (2 * PI / 360)))) * 180 / (PI * 15);
  //  // 计算赤纬δ = 赤纬。天赤道以北为正，以南为负。
  Astro_HUD_DEC = asin(sin(GPS_Latitude * 2 * PI / 360) * sin(Encoder_Altitude_Radian) - cos(GPS_Latitude * (2 * PI / 360)) * cos(Encoder_Altitude_Radian) * cos(Encoder_Azimuth_Radian)) * 360 / (2 * PI);
    Serial.print("\t RA:\t");
      Serial.print(Astro_HUD_RA);
 
  
  //Serial.print("\t RA in Degree: \t");
  //Serial.print(Astro_HUD_RA * 15);
    Serial.print("\t DEC: \t");
    Serial.println(Astro_HUD_DEC);

  // 以下获取赤经、赤纬的独立显示值
  Mod_RA_HH = int(Astro_HUD_RA);
  Mod_RA_MM = int(fmod(Astro_HUD_RA, 1) * 60);
  Mod_RA_SS = int((Astro_HUD_RA - Mod_RA_HH - Mod_RA_MM / 60) * 60);
  Mod_DEC_DD = int(Astro_HUD_DEC);
  Mod_DEC_MM = int(fmod(abs(Astro_HUD_DEC), 1) * 60);
  Mod_DEC_SS = int((abs(Astro_HUD_DEC) - abs(Mod_DEC_DD) - Mod_DEC_MM / 60) * 60);

  // 串口输出方位角、高度角等信息
  //  Serial.print(Mod_RA_HH);
  //  Serial.print(" h ");
  //  Serial.print(Mod_RA_MM);
  //  Serial.print(" m ");
  //  Serial.print(Mod_RA_SS);
  //  Serial.print(" s");
  //  Serial.print(Mod_DEC_DD);
  //  Serial.print(" d ");
  //  Serial.print(Mod_DEC_MM);
  //  Serial.print(" m ");
  //  Serial.print(Mod_DEC_SS);
  //  Serial.print(" s");

  // 读取Stellarium数据
 
//     while (Serial.available() > 0)
//     {
//     inChar = Serial.read();
//     Stellarium += String(inChar);
//     delay(5);
//     }
//     // 向Stellarium传送RA赤经值
//     if (Stellarium == "#:GR#")
//     {
//     if (Mod_RA_HH < 10)
//     {
//       Serial.print("0");
//     }
//     Serial.print(Mod_RA_HH);
//     Serial.print(":");
//     if (Mod_RA_MM < 10)
//     {
//       Serial.print("0");
//     }
//     Serial.print(Mod_RA_MM);
//     Serial.print(":00#");
//     Stellarium = "";
//     }
//     // 向Stellarium传送DEC赤纬值
//     if (Stellarium == "#:GD#")
//     {
//     if ((Mod_DEC_DD >= 0 && Mod_DEC_DD < 10))
//     {
//       Serial.print("+0");
//       Serial.print(Mod_DEC_DD);
//     }
//     else if (Mod_DEC_DD >= 10)
//     {
//       Serial.print("+");
//       Serial.print(Mod_DEC_DD);
//     }
//     else if ((Mod_DEC_DD < 0) && abs(Mod_DEC_DD) < 10)
//     {
//       Serial.print("-0");
//       Serial.print(abs(Mod_DEC_DD));
//     }
//     else
//     {
//       Serial.print(Mod_DEC_DD);
//     }
//     Serial.print("*");
//     if (Mod_DEC_MM < 10)
//     {
//       Serial.print("0");
//     }
//     Serial.print(Mod_DEC_MM);
//     Serial.print(":00#");
//     Stellarium = "";
//     }
}
}
