#define rainSensorPin A1   // Use A1 (or A0 if free)

// Raw & percentage values
int rainRawValue;
int rainPercentageValue;

// Thresholds (0–100%)
int lightThreshold  = 20;
int mediumThreshold = 50;
int heavyThreshold  = 80;

// Time + precipitation logic
bool  isRainingNow          = false;
unsigned long rainStartMs   = 0;   // when current rain started
unsigned long lastSampleMs  = 0;   // last time we calculated
float currentRainMinutes    = 0.0; // continuous rain duration (minutes)
float rainIndex             = 0.0; // “fake” precipitation index

void setup() {
  Serial.begin(9600);   // To ESP8266
}

// Helper to map sensor reading to 0–100% wetness
int calcRainPercent(int raw) {
  // Many rain modules: 1023 = dry, ~300 = very wet.
  int percent = map(raw, 1023, 300, 0, 100);
  if (percent < 0)   percent = 0;
  if (percent > 100) percent = 100;
  return percent;
}

void loop() {
  unsigned long now = millis();

  // Sample every 1 second
  if (now - lastSampleMs < 1000) {
    return;
  }
  lastSampleMs = now;

  // 1) Read sensor
  rainRawValue        = analogRead(rainSensorPin);
  rainPercentageValue = calcRainPercent(rainRawValue);

  // 2) Decide intensity level
  int levelCode = 0;      // 0=NONE,1=LIGHT,2=MEDIUM,3=HEAVY
  if (rainPercentageValue >= heavyThreshold) {
    levelCode = 3;
  } else if (rainPercentageValue >= mediumThreshold) {
    levelCode = 2;
  } else if (rainPercentageValue >= lightThreshold) {
    levelCode = 1;
  } else {
    levelCode = 0;
  }

  const char* levelText = "NONE";
  if (levelCode == 1) levelText = "LIGHT";
  if (levelCode == 2) levelText = "MEDIUM";
  if (levelCode == 3) levelText = "HEAVY";

  // 3) Time and Rain Index logic
  // minutes since last sample
  static unsigned long prevMs = now;
  float dtMinutes = (now - prevMs) / 60000.0;
  prevMs = now;

  if (levelCode > 0) {
    // Raining now
    if (!isRainingNow) {
      // New rain event started
      isRainingNow = true;
      rainStartMs  = now;
    }

    currentRainMinutes = (now - rainStartMs) / 60000.0;

    // Intensity factor for index (tune as you want)
    float factor = 0.0;
    if (levelCode == 1) factor = 0.1;  // LIGHT
    if (levelCode == 2) factor = 0.3;  // MEDIUM
    if (levelCode == 3) factor = 0.6;  // HEAVY

    // Increase “precipitation index”
    rainIndex += factor * dtMinutes;   // more time + stronger rain = larger index
  } else {
    // Not raining now
    isRainingNow       = false;
    currentRainMinutes = 0.0;  // reset continuous duration
  }

  // 4) Debug to Serial Monitor
  Serial.print("Rain raw=");
  Serial.print(rainRawValue);
  Serial.print(" | %=");
  Serial.print(rainPercentageValue);
  Serial.print(" | level=");
  Serial.print(levelText);
  Serial.print(" | nowMin=");
  Serial.print(currentRainMinutes, 1);
  Serial.print(" | index=");
  Serial.println(rainIndex, 2);

  // 5) 🔥 SEND CLEAN LINE TO ESP8266
  // Format:
  // RAIN,wet=45,level=MEDIUM,nowMin=3.5,index=1.20
  Serial.print("RAIN,wet=");
  Serial.print(rainPercentageValue);
  Serial.print(",level=");
  Serial.print(levelText);
  Serial.print(",nowMin=");
  Serial.print(currentRainMinutes, 1);
  Serial.print(",index=");
  Serial.println(rainIndex, 2);
}
