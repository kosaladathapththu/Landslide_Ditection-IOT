<div align="center">

# 🌋 Smart Landslide Simulation & Early Warning System

### 📡 IoT-Based Real-Time Slope Monitoring | Lab-Scale Landslide Model

[![IoT](https://img.shields.io/badge/IoT-Enabled-blue?style=for-the-badge&logo=internetofthings)](https://github.com/kosaladathapththu)
[![ESP32](https://img.shields.io/badge/ESP32-Powered-red?style=for-the-badge&logo=espressif)](https://github.com/kosaladathapththu)
[![Status](https://img.shields.io/badge/Status-Active-success?style=for-the-badge)](https://github.com/kosaladathapththu)

**🎓 Higher National Diploma in Software Engineering — IoT Module**  
**🏫 National Institute of Business Management (NIBM) | Batch: HDSE 25.1F**

[📖 Documentation](#-project-overview) • [🎯 Features](#-key-features) • [🔧 Hardware](#-components-used) • [👥 Team](#-our-team)

---

</div>

## 🚨 Problem Statement

Sri Lanka faces **frequent landslides** in high-risk regions like **Hatton, Nuwara Eliya, and Badulla**, especially during monsoon seasons. Current systems rely primarily on rainfall measurements and **don't directly monitor soil conditions**. 

**Our Solution?** A smart IoT-enabled lab simulator that monitors **soil moisture, tilt, and vibration** in real-time to predict slope instability before disaster strikes.

---

## 🌍 Project Overview

This project builds a **laboratory-scale IoT landslide simulator** to understand slope failure behavior and generate **early warnings** using real-time sensor monitoring. It serves as both a **research tool** and **educational platform** for students and researchers to safely study landslide dynamics.

### 🎯 Main Objective

Develop a **cost-effective IoT-based Smart Landslide Simulator & Early Warning System** that tracks soil, rainfall, and movement in real-time to detect and forecast slope instability in a controlled laboratory environment.

### 📌 Specific Objectives

- ✅ Build a **mini soil slope model** with controlled artificial rainfall
- ✅ Monitor slope continuously using **IoT sensors** (moisture, tilt, vibration, rainfall)
- ✅ Implement **wireless data transmission** via Wi-Fi/LoRa
- ✅ Create a **real-time dashboard** with Safe/Warning/Critical indicators
- ✅ Analyze sensor data to determine **threshold values** for landslide risk
- ✅ Test system under **various soil and rainfall conditions**
- ✅ Provide a **safe learning platform** for academic institutions

---

## ✨ Key Features

<table>
<tr>
<td width="50%">

### 🌧️ Rainfall Simulation
- Automated water pump system
- Controlled rainfall intensity
- Simulates real monsoon conditions

### 📊 Real-Time Monitoring
- Live sensor data streaming
- Continuous slope stability tracking
- Instant anomaly detection

</td>
<td width="50%">

### 🚨 Smart Alerts
- Color-coded warning system
- LED indicators (🟢🟡🔴)
- Buzzer for critical conditions
- Dashboard notifications

### 📈 Data Visualization
- Interactive web dashboard
- Historical data logging
- Graphical trend analysis

</td>
</tr>
</table>

---

## 🔧 Components Used

| Component | Purpose | Quantity |
|-----------|---------|----------|
| 🤖 **ESP32** | Central IoT controller with Wi-Fi | 1 |
| 🌱 **Soil Moisture Sensor** | Detects soil water content | 2 |
| ⚡ **Vibration Sensor (SW-420)** | Detects ground vibrations | 1 |
| ↗️ **Accelerometer** | Monitors slope angle changes | 1 |
| 🌡️ **DHT22 Sensor** | Temperature & humidity monitoring | 1 |
| 🚰 **Mini Water Pump + Nozzle** | Artificial rainfall simulation | 1 |
| 🔌 **Relay Module** | Controls pump automatically | 1 |
| 🔴🟡🟢 **LEDs** | Visual status indicators | 3 |
| 🔔 **Buzzer Module** | Audio alert for critical state | 1 |
| 📦 **Acrylic/Wooden Slope Box** | Lab-scale slope model | 1 |
| 📡 **Wi-Fi Router** | Wireless data transmission | 1 |
| 🔋 **12V DC Power Supply** | Powers all components | 1 |

**💰 Total Estimated Cost:** ~LKR 20,950

---

## 🎨 System Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    🌧️ RAINFALL SIMULATION                    │
│                      (Water Pump System)                     │
└──────────────────────┬──────────────────────────────────────┘
                       │
                       ▼
┌─────────────────────────────────────────────────────────────┐
│                   📊 SENSOR LAYER                            │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐   │
│  │ Moisture │  │   Tilt   │  │ Vibration│  │ Temp/Hum │   │
│  │  Sensor  │  │  Sensor  │  │  Sensor  │  │  Sensor  │   │
│  └──────────┘  └──────────┘  └──────────┘  └──────────┘   │
└──────────────────────┬──────────────────────────────────────┘
                       │
                       ▼
┌─────────────────────────────────────────────────────────────┐
│              🤖 ESP32 MICROCONTROLLER                        │
│         (Data Processing & Wi-Fi Communication)              │
└──────────────────────┬──────────────────────────────────────┘
                       │
                       ▼
┌─────────────────────────────────────────────────────────────┐
│              ☁️ IoT CLOUD PLATFORM                          │
│         (Blynk / ThingSpeak / Custom Server)                 │
└──────────────────────┬──────────────────────────────────────┘
                       │
           ┌───────────┴───────────┐
           ▼                       ▼
┌──────────────────────┐  ┌──────────────────────┐
│   🖥️ WEB DASHBOARD   │  │  🚨 ALERT SYSTEM     │
│  • Live Data View    │  │  • LED Indicators    │
│  • Graphs & Charts   │  │  • Buzzer Alarm      │
│  • Status Monitor    │  │  • Notifications     │
└──────────────────────┘  └──────────────────────┘
```

---

## 🚦 System States

<div align="center">

| Status | Condition | Moisture | Tilt | Vibration | Indicator |
|--------|-----------|----------|------|-----------|-----------|
| 🟢 **Safe** | Normal operation | Low | Stable | None | Green LED |
| 🟡 **Warning** | Moderate risk | High | Slight change | Minor | Yellow LED |
| 🔴 **Critical** | High danger | Very High | Significant | Detected | Red LED + Buzzer |

</div>

---

## 📱 Dashboard Features

<table>
<tr>
<td width="50%">

### 📊 Real-Time Display
- Live sensor data from all IoT devices
- Soil moisture percentage
- Slope angle monitoring
- Vibration intensity levels
- Temperature & humidity readings

### 📈 Data Visualization
- Time-series graphs
- Historical data trends
- Comparative analysis charts

</td>
<td width="50%">

### 🎨 Status Indicators
- **🟢 Safe:** Normal conditions
- **🟡 Warning:** Medium risk
- **🔴 Critical:** High danger

### 🔔 Alert System
- On-screen pop-up notifications
- LED visual alerts
- Buzzer audio warnings
- Email/SMS notifications (future)

### ⚙️ Control Panel
- Manual pump control
- Alert threshold settings
- System reset options
- Connectivity status monitor

</td>
</tr>
</table>

---

## 👥 Our Team

<div align="center">

### 🌟 HDSE 25.1F — Team Landslide

</div>

<table>
<tr>
<td align="center" width="20%">
<img src="https://via.placeholder.com/150/FF6B6B/FFFFFF?text=KDA" width="100px" style="border-radius: 50%"/>
<br />
<strong>⭐ Kosala D. Athapaththu</strong>
<br />
<sub>Team Lead / System Developer</sub>
<br />
<code>COHNDSE25.1-001</code>
<br />
<a href="https://github.com/kosaladathapththu">
<img src="https://img.shields.io/badge/GitHub-100000?style=flat&logo=github&logoColor=white" />
</a>
<a href="https://www.linkedin.com/in/kosala-d-athapaththu-a453b9248/">
<img src="https://img.shields.io/badge/LinkedIn-0077B5?style=flat&logo=linkedin&logoColor=white" />
</a>
</td>

<td align="center" width="20%">
<img src="https://via.placeholder.com/150/4ECDC4/FFFFFF?text=MIR" width="100px" style="border-radius: 50%"/>
<br />
<strong>⭐ M.I. Rushdee</strong>
<br />
<sub>Hardware & Sensors</sub>
<br />
<code>COHNDSE25.1-003</code>
<br />
<a href="#">
<img src="https://img.shields.io/badge/GitHub-100000?style=flat&logo=github&logoColor=white" />
</a>
<a href="#">
<img src="https://img.shields.io/badge/LinkedIn-0077B5?style=flat&logo=linkedin&logoColor=white" />
</a>
</td>

<td align="center" width="20%">
<img src="https://via.placeholder.com/150/95E1D3/FFFFFF?text=GNK" width="100px" style="border-radius: 50%"/>
<br />
<strong>⭐ G.N.A. Kodagoda</strong>
<br />
<sub>Cloud Dashboard & UI</sub>
<br />
<code>COHNDSE25.1-043</code>
<br />
<a href="#">
<img src="https://img.shields.io/badge/GitHub-100000?style=flat&logo=github&logoColor=white" />
</a>
<a href="#">
<img src="https://img.shields.io/badge/LinkedIn-0077B5?style=flat&logo=linkedin&logoColor=white" />
</a>
</td>

<td align="center" width="20%">
<img src="https://via.placeholder.com/150/F38181/FFFFFF?text=MFR" width="100px" style="border-radius: 50%"/>
<br />
<strong>⭐ M.F.M. Rizni</strong>
<br />
<sub>Testing & Calibration</sub>
<br />
<code>COHNDSE25.1-067</code>
<br />
<a href="#">
<img src="https://img.shields.io/badge/GitHub-100000?style=flat&logo=github&logoColor=white" />
</a>
<a href="#">
<img src="https://img.shields.io/badge/LinkedIn-0077B5?style=flat&logo=linkedin&logoColor=white" />
</a>
</td>

<td align="center" width="20%">
<img src="https://via.placeholder.com/150/AA96DA/FFFFFF?text=TNV" width="100px" style="border-radius: 50%"/>
<br />
<strong>⭐ T.N. Vithanachchi</strong>
<br />
<sub>Documentation & Analysis</sub>
<br />
<code>COHNDSE25.1-084</code>
<br />
<a href="#">
<img src="https://img.shields.io/badge/GitHub-100000?style=flat&logo=github&logoColor=white" />
</a>
<a href="#">
<img src="https://img.shields.io/badge/LinkedIn-0077B5?style=flat&logo=linkedin&logoColor=white" />
</a>
</td>
</tr>
</table>

<div align="center">
<sub>👉 <i>Team members: Update your GitHub and LinkedIn links above!</i></sub>
</div>

---

## 📁 Project Structure

```
Smart-Landslide-Simulator/
│
├── 📄 README.md                    # This file
├── 📄 proposal.pdf                 # Official project proposal
│
├── 📂 src/
│   ├── main_code.ino              # ESP32 main control code
│   ├── sensor_test.ino            # Individual sensor testing
│   └── calibration.ino            # Sensor calibration scripts
│
├── 📂 hardware/
│   ├── circuit_diagram.png        # Wiring diagram
│   ├── pcb_design.pdf             # PCB layout (if applicable)
│   └── slope_model_design.png     # Physical model design
│
├── 📂 dashboard/
│   ├── index.html                 # Web dashboard UI
│   ├── script.js                  # Dashboard logic
│   └── style.css                  # Dashboard styling
│
├── 📂 data/
│   └── sample_readings.csv        # Sample sensor data logs
│
├── 📂 docs/
│   ├── user_manual.pdf            # System user guide
│   ├── technical_report.pdf       # Detailed technical documentation
│   └── presentation.pptx          # Project presentation
│
└── 📂 tests/
    └── integration_test.ino       # Full system testing
```

---

## 🚀 Getting Started

### Prerequisites

- **Arduino IDE** (v1.8.19 or higher)
- **ESP32 Board Package** installed
- **Required Libraries:**
  - `WiFi.h`
  - `DHT.h`
  - `BlynkSimpleEsp32.h` (if using Blynk)
  - `ThingSpeak.h` (if using ThingSpeak)

### Installation

1. **Clone the repository**
   ```bash
   git clone https://github.com/kosaladathapththu/smart-landslide-simulator.git
   cd smart-landslide-simulator
   ```

2. **Open Arduino IDE**
   - Load `src/main_code.ino`
   - Configure your Wi-Fi credentials
   - Update IoT platform API keys

3. **Upload to ESP32**
   - Select **ESP32 Dev Module** as board
   - Choose correct COM port
   - Click Upload

4. **Setup Dashboard**
   - Open `dashboard/index.html` in browser, OR
   - Configure Blynk/ThingSpeak dashboard using provided templates

### Testing

Run individual sensor tests before full deployment:
```bash
# Test soil moisture sensor
Open: tests/sensor_test.ino

# Test complete system
Open: tests/integration_test.ino
```

---

## 📅 Project Timeline

| Week | Milestone | Status |
|------|-----------|--------|
| **Week 1** | Component procurement & sensor testing | ✅ |
| **Week 2** | Hardware assembly & slope box construction | 🔄 |
| **Week 3** | Dashboard development & IoT integration | ⏳ |
| **Week 4** | System testing, documentation & presentation | ⏳ |

---

## 🎓 Learning Outcomes

This project enables students to:

- 🔹 Understand **IoT sensor integration** with microcontrollers
- 🔹 Learn **real-time data processing** and wireless communication
- 🔹 Explore **environmental monitoring** and early warning systems
- 🔹 Gain hands-on experience with **ESP32 programming**
- 🔹 Develop skills in **dashboard creation** and data visualization
- 🔹 Study **landslide dynamics** in a safe, controlled environment

---

## 🔮 Future Enhancements

- 🔜 **Machine Learning Integration** — AI-based risk prediction models
- 🔜 **SMS Alert System** — Automatic notifications via GSM module
- 🔜 **LoRa Communication** — Long-range wireless for remote areas
- 🔜 **Solar Power** — Outdoor deployment capability
- 🔜 **Mobile App** — iOS/Android real-time monitoring
- 🔜 **Multi-Slope Network** — Connect multiple sensors across larger areas
- 🔜 **Weather API Integration** — Real-time rainfall data correlation

---

## 🤝 Contributing

We welcome contributions! Here's how you can help:

1. 🍴 Fork the repository
2. 🌿 Create a feature branch (`git checkout -b feature/AmazingFeature`)
3. 💾 Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. 📤 Push to the branch (`git push origin feature/AmazingFeature`)
5. 🔃 Open a Pull Request

---

## 📜 License

This project is developed as part of academic coursework at NIBM.  
For educational and research purposes.

---

## 🙏 Acknowledgments

Special thanks to:

- 💛 **School of Computing and Engineering — NIBM**
- 💛 **IoT Module Lecturers** for guidance and support
- 💛 All team members for their dedication and hard work
- 💛 Open-source community for libraries and resources

---

## 📞 Contact & Support

<div align="center">

### 📧 Have Questions?

**Project Lead:** Kosala D. Athapaththu  
**Institution:** National Institute of Business Management  
**Email:** [Insert Email]

<br>

### ⭐ Support This Project

If this project helped you or you find it interesting, please **star this repository**! 🌟

[![Star this repo](https://img.shields.io/github/stars/kosaladathapththu/smart-landslide-simulator?style=social)](https://github.com/kosaladathapththu)

</div>

---

<div align="center">

**🌋 Built with ❤️ by Team Landslide — HDSE 25.1F**

**Making Communities Safer Through Technology 🛡️**

[![NIBM](https://img.shields.io/badge/NIBM-Colombo-blue?style=for-the-badge)](https://nibm.lk)
[![IoT](https://img.shields.io/badge/IoT-Project-orange?style=for-the-badge)](https://github.com/kosaladathapththu)

</div>
