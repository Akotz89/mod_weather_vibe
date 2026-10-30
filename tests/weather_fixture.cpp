#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <map>
#include <string>
#include <sstream>
using uint32=uint32_t;int checks=0;
#define CHECK(x) do{++checks;if(!(x)){std::cerr<<"failed line "<<__LINE__<<": "<<#x<<"\n";return 1;}}while(false)
bool close(float a,float b){return std::fabs(a-b)<0.000001f;}
enum class DayPart{MORNING,AFTERNOON,EVENING,NIGHT,COUNT};using WeatherState=uint32;constexpr WeatherState WEATHER_STATE_FINE=0;
struct Range{float min,max;};
struct Config{std::string value;template<class T>T GetOption(std::string const&,T const&def){return value.empty()?def:value;}}cfg;auto sConfigMgr=&cfg;
struct WeatherVibeCore{std::map<uint32,Range>m_stateRanges[4];float raw=-1;WeatherState state=0;uint32 zone=0;float percent=0;
 static constexpr WeatherState kAcceptedStates[]={0,1};
 static char const* DayPartTokenUpper(DayPart){return "MORNING";} static char const* ConfigStateToken(WeatherState){return "Fine";}
 void LoadIntensityRangesConfig();
 float MapPercentToRawGrade(DayPart,WeatherState,float)const;Range ParseRangePair(std::string const&,Range);
 bool PushWeatherPercent(uint32,WeatherState,float);bool PushWeatherDebug(uint32,WeatherState,float);
 DayPart GetCurrentDayPart(){return DayPart::MORNING;}bool PushWeatherToClient(uint32 z,WeatherState s,float r,float p=0){zone=z;state=s;raw=r;percent=p;return true;}};
@BODY@
int main(){WeatherVibeCore w;w.m_stateRanges[0][0]={0,.3f};
 CHECK(close(w.MapPercentToRawGrade(DayPart::MORNING,0,0),.3f));CHECK(close(w.MapPercentToRawGrade(DayPart::MORNING,0,.5f),.15f));CHECK(close(w.MapPercentToRawGrade(DayPart::MORNING,0,1),0));
 w.m_stateRanges[0][0]={.2f,.8f};CHECK(close(w.MapPercentToRawGrade(DayPart::MORNING,0,.25f),.65f));CHECK(close(w.MapPercentToRawGrade(DayPart::MORNING,0,-1),.8f));CHECK(close(w.MapPercentToRawGrade(DayPart::MORNING,0,2),.2f));
 for(uint32 state=1;state<=11;++state){w.m_stateRanges[0][state]={.1f,.9f};CHECK(close(w.MapPercentToRawGrade(DayPart::MORNING,state,0),.1f));CHECK(close(w.MapPercentToRawGrade(DayPart::MORNING,state,.25f),.3f));CHECK(close(w.MapPercentToRawGrade(DayPart::MORNING,state,1),.9f));}
 CHECK(close(w.MapPercentToRawGrade(DayPart::NIGHT,0,.5f),.15f));CHECK(close(w.MapPercentToRawGrade(DayPart::NIGHT,0,1),0));CHECK(close(w.MapPercentToRawGrade(DayPart::NIGHT,1,0),.3f));
 cfg.value="";w.LoadIntensityRangesConfig();CHECK(close(w.MapPercentToRawGrade(DayPart::NIGHT,0,1),0));CHECK(close(w.MapPercentToRawGrade(DayPart::NIGHT,0,0),.3f));CHECK(close(w.MapPercentToRawGrade(DayPart::NIGHT,1,0),.3f));
 cfg.value="invalid";w.LoadIntensityRangesConfig();CHECK(close(w.MapPercentToRawGrade(DayPart::NIGHT,0,1),0));CHECK(close(w.MapPercentToRawGrade(DayPart::NIGHT,0,0),.3f));CHECK(close(w.MapPercentToRawGrade(DayPart::NIGHT,1,0),.3f));
 for (auto const* value : {"nan,.3", "0,nan", "inf,.3", "0,-inf"}) {cfg.value=value;w.LoadIntensityRangesConfig();CHECK(close(w.MapPercentToRawGrade(DayPart::NIGHT,0,1),0));CHECK(close(w.MapPercentToRawGrade(DayPart::NIGHT,0,0),.3f));CHECK(close(w.MapPercentToRawGrade(DayPart::NIGHT,1,0),.3f));CHECK(close(w.MapPercentToRawGrade(DayPart::NIGHT,1,1),1));}
 cfg.value=".2,.8";w.LoadIntensityRangesConfig();CHECK(close(w.MapPercentToRawGrade(DayPart::NIGHT,0,1),.2f));CHECK(close(w.MapPercentToRawGrade(DayPart::NIGHT,0,0),.8f));w.m_stateRanges[0][0]={.2f,.8f};
 cfg.value=".8,.2";auto r=w.ParseRangePair("key",{.3f,1});CHECK(close(r.min,.2f)&&close(r.max,.8f));cfg.value="-1,2";r=w.ParseRangePair("key",{.3f,1});CHECK(r.min==0&&r.max==1);cfg.value="invalid";r=w.ParseRangePair("key",{.3f,1});CHECK(close(r.min,.3f)&&r.max==1);
 CHECK(w.PushWeatherPercent(42,0,25));CHECK(close(w.raw,.65f)&&w.percent==25&&w.zone==42);CHECK(w.PushWeatherDebug(42,0,.75f));CHECK(close(w.raw,.75f));CHECK(w.PushWeatherDebug(42,0,2));CHECK(w.raw==1);CHECK(w.PushWeatherDebug(42,0,-1));CHECK(w.raw==0);
 std::cout<<checks<<" Weather checks\n";
}
