  // ------------------------------------------------
  // ACCELEROMETER LINES
  //   Format:
  //   ACCEL1,tilt=23.45,tiltLevel=MID,vib=0.12,vibLevel=LOW
  // ------------------------------------------------
  if (line.startsWith("ACCEL")) {
    int accelId = 0;   // 1,2,3

    if (line.startsWith("ACCEL1"))      accelId = 1;
    else if (line.startsWith("ACCEL2")) accelId = 2;
    else if (line.startsWith("ACCEL3")) accelId = 3;
    else {
      Serial.println("⚠ Unknown ACCEL ID");
      return;
    }

    float tiltValue = 0;
    String tiltLevel;
    float vibValue  = 0;
    String vibLevel;

    // tilt=
    int tIndex = line.indexOf("tilt=");
    if (tIndex != -1) {
      int commaIndex = line.indexOf(",", tIndex);
      String tStr;
      if (commaIndex == -1) {
        tStr = line.substring(tIndex + 5);
      } else {
        tStr = line.substring(tIndex + 5, commaIndex);
      }
      tiltValue = tStr.toFloat();
    }

    // tiltLevel=
    int tlIndex = line.indexOf("tiltLevel=");
    if (tlIndex != -1) {
      int commaIndex = line.indexOf(",", tlIndex);
      if (commaIndex == -1) {
        tiltLevel = line.substring(tlIndex + 10);
      } else {
        tiltLevel = line.substring(tlIndex + 10, commaIndex);
      }
    }

    // vib=
    int vIndex = line.indexOf("vib=");
    if (vIndex != -1) {
      int commaIndex = line.indexOf(",", vIndex);
      String vStr;
      if (commaIndex == -1) {
        vStr = line.substring(vIndex + 4);
      } else {
        vStr = line.substring(vIndex + 4, commaIndex);
      }
      vibValue = vStr.toFloat();
    }

    // vibLevel=
    int vlIndex = line.indexOf("vibLevel=");
    if (vlIndex != -1) {
      // last field, no need to search comma
      vibLevel = line.substring(vlIndex + 9);
    }

    Serial.print("ACCEL");
    Serial.print(accelId);
    Serial.print(" → tilt=");
    Serial.print(tiltValue);
    Serial.print(" (");
    Serial.print(tiltLevel);
    Serial.print("), vib=");
    Serial.print(vibValue, 3);
    Serial.print(" (");
    Serial.print(vibLevel);
    Serial.println(")");

    // Firebase base path
    String basePath = "device1/accelerometer";
    basePath += String(accelId);

    // Upload to Firebase
    Firebase.RTDB.setFloat(&fbdo, basePath + "/tilt", tiltValue);
    Firebase.RTDB.setString(&fbdo, basePath + "/tiltLevel", tiltLevel);
    Firebase.RTDB.setFloat(&fbdo, basePath + "/vibration", vibValue);
    Firebase.RTDB.setString(&fbdo, basePath + "/vibrationLevel", vibLevel);

    return;
  }
