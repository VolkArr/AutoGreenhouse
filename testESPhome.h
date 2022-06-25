#include "esphome.h"
#include "/config/esphome/DHTesp.h"


static const char* TAG = "MyCustomComponent";
hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
static const short main_temperature = 25;


#define W_OPEN  23
#define W_CLOSE  22
#define EN  12
#define HG  32
#define DHT  16

struct PWMconfig{
    int freq = 5000;
    int ledChannel = 0;
    int resolution = 8;
};

void IRAM_ATTR onTimer() {
    portENTER_CRITICAL_ISR(&timerMux);
    ledcWrite(0, 0);
    timerStop(timer);
    portEXIT_CRITICAL_ISR(&timerMux);
}

class MyCustomSensor : public PollingComponent {
    public:
    MyCustomSensor() : PollingComponent(1000), old_temperature{0}, old_humadity{0}, old_humadity_ground(-1), state{false}{}

    Sensor *temperature_sensor = new Sensor();
    Sensor *humidity_sensor = new Sensor();
    Sensor *humidity_ground_sensor = new Sensor();

    // float get_setup_priority() const override { return esphome::setup_priority::WIFI; }

    void setup() override {
        timer = timerBegin(0, 80, true);
        timerAttachInterrupt(timer, &onTimer, true);
        timerAlarmWrite(timer, 60000000, true);
        timerAlarmEnable(timer);
        timerStop(timer);
        ledcSetup(pwmConf.ledChannel, pwmConf.freq, pwmConf.resolution);
        dht.setup(DHT, DHTesp::DHT22);
        ledcAttachPin(EN, pwmConf.ledChannel);
        pinMode(W_OPEN, OUTPUT);
        pinMode(W_CLOSE, OUTPUT);
        pinMode(HG, INPUT);
        adcAttachPin(HG);

        const float temperature = dht.getTemperature();
        if(temperature >= main_temperature){
            state = true;
            portENTER_CRITICAL(&timerMux);
            timerStart(timer);
            ledcWrite(pwmConf.ledChannel, 255);
            digitalWrite(W_OPEN, LOW);
            digitalWrite(W_CLOSE, HIGH);
            portEXIT_CRITICAL(&timerMux);
            temperature_sensor->publish_state(temperature);
            id(window_state).publish_state(true);
        }
        else{
            state = false;
            portENTER_CRITICAL(&timerMux);
            timerStart(timer);
            ledcWrite(pwmConf.ledChannel, 255);
            digitalWrite(W_CLOSE, LOW);
            digitalWrite(W_OPEN, HIGH);
            portEXIT_CRITICAL(&timerMux);
            temperature_sensor->publish_state(temperature);
            id(window_state).publish_state(false);
        }
    }        
    
    void update() override {

        const float temperature = dht.getTemperature();
        if((temperature - old_temperature < 50 && temperature - old_temperature > -50)
        && (temperature != old_temperature)){
                        old_temperature = temperature;
                        if((temperature >= main_temperature + id(delta_temperature).state) && !state){
                            state = true;
                            portENTER_CRITICAL(&timerMux);
                            timerStart(timer);
                            ledcWrite(pwmConf.ledChannel, 255);
                            digitalWrite(W_OPEN, LOW);
                            digitalWrite(W_CLOSE, HIGH);
                            portEXIT_CRITICAL(&timerMux);
                        }
                        temperature_sensor->publish_state(temperature);
                        id(window_state).publish_state(true);
        }
            
        if( (temperature - old_temperature < 50 && temperature - old_temperature > -50)
        && (temperature != old_temperature)){
                        old_temperature = temperature;
                        if((temperature <= main_temperature - id(delta_temperature).state) && state){
                            state = false;
                            portENTER_CRITICAL(&timerMux);
                            timerStart(timer);
                            ledcWrite(pwmConf.ledChannel, 255);
                            digitalWrite(W_CLOSE, LOW);
                            digitalWrite(W_OPEN, HIGH);
                            portEXIT_CRITICAL(&timerMux);
                        }
                        temperature_sensor->publish_state(temperature);
                        id(window_state).publish_state(false); 
        }

        const float humadity =  dht.getHumidity();
        if((humadity - old_humadity < 100 && humadity - old_humadity > -100)
        && (humadity != old_humadity)){
            old_humadity = humadity;
            humidity_sensor->publish_state(humadity);
        } 

        const float humadity_ground = digitalRead(HG);
        humidity_ground_sensor->publish_state(humadity_ground);
        // if((humidity_ground - old_humadity_ground < 50 && humidity_ground - old_humadity_ground > -50)
        // && (humidity_ground != old_humadity_ground)){
        //     old_humadity_ground = humidity_ground;
        //     humidity_ground_sensor->publish_state(humidity_ground);
        // } 

    }

    private:
        
    float old_temperature;
    float old_humadity;
    float old_humadity_ground;
    
    bool state;
    
    DHTesp dht;
    PWMconfig pwmConf;

};