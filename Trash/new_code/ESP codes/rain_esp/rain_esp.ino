  // ------------------------------------------------
  // RAIN SENSOR LINE
  //   Format: RAIN,wet=45,level=MEDIUM,nowMin=3.5,index=1.20
  // ------------------------------------------------
  if (line.startsWith("RAIN")) {
    float wetValue      = 0;
    String levelValue   = "";
    float nowMinValue   = 0;
    float indexValue    = 0;

    // wet=
    int wIndex = line.indexOf("wet=");
    if (wIndex != -1) {
      int commaIndex = line.indexOf(",", wIndex);
      String wStr;
      if (commaIndex == -1) {
        wStr = line.substring(wIndex + 4);
      } else {
        wStr = line.substring(wIndex + 4, commaIndex);
      }
      wetValue = wStr.toFloat();
    }

    // level=
    int levelIndex = line.indexOf("level=");
    if (levelIndex != -1) {
      int commaIndex = line.indexOf(",", levelIndex);
      if (commaIndex == -1) {
        levelValue = line.substring(levelIndex + 6);
      } else {
        levelValue = line.substring(levelIndex + 6, commaIndex);
      }
    }

    // nowMin=
    int nowIndex = line.indexOf("nowMin=");
    if (nowIndex != -1) {
      int commaIndex = line.indexOf(",", nowIndex);
      String nStr;
      if (commaIndex == -1) {
        nStr = line.substring(nowIndex + 7);
      } else {
        nStr = line.substring(nowIndex + 7, commaIndex);
      }
      nowMinValue = nStr.toFloat();
    }

    // index=
    int idxIndex = line.indexOf("index=");
    if (idxIndex != -1) {
      String iStr = line.substring(idxIndex + 6);
      indexValue = iStr.toFloat();
    }

    Serial.print("RAIN Parsed → wet=");
    Serial.print(wetValue);
    Serial.print(" | level=");
    Serial.print(levelValue);
    Serial.print(" | nowMin=");
    Serial.print(nowMinValue);
    Serial.print(" | index=");
    Serial.println(indexValue);

    // Upload to Firebase
    Firebase.RTDB.setFloat(&fbdo, "device1/rain/wet", wetValue);
    Firebase.RTDB.setString(&fbdo, "device1/rain/level", levelValue);
    Firebase.RTDB.setFloat(&fbdo, "device1/rain/current_minutes", nowMinValue);
    Firebase.RTDB.setFloat(&fbdo, "device1/rain/rain_index", indexValue);

    return; // Done handling RAIN line
  }
