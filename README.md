# btex-air-sensor

BardBox Air Node is an ESP32-based environmental monitoring platform designed for air quality sensing, environmental data collection, and creative applications.

The system supports particulate, climate, and gas sensors, local SD card logging, wireless data upload, and onboard TFT visualization. It serves as a modular sensing node within the broader BardBox ecosystem.

Current sensors include:

- PMS5003 particulate sensor
- BME688 temperature, humidity, pressure, and gas sensor
- SCD-30 CO₂ sensor
- Additional VOC and gas sensors (experimental)

Project Status: Prototype Development

## BardBox Operations

Preview configuration synchronization after pulling source updates:

```bash
python3 scripts/sync_app_config.py
```

Apply the reviewed merge with:

```bash
python3 scripts/sync_app_config.py --write
```

The canonical BardBox synchronizer recursively adds new example fields while
preserving deployment values, secrets, and unknown local fields.
