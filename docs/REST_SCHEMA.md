# REST Schema

## Camera Connection

### POST /v1/camera
Connect a new camera.

Request:
```json
{
    "ip": "string",
    "password": "string"
}
```

Response: 201

### GET /v1/camera
Get list of connected cameras.

Response: 200
```json
{
    "cameras": [
        {
            "serial": "string",
            "ip": "string",
            "model": "string",
            "recording": null
        }
    ]
}
```

## Camera Recording

### POST /v1/camera/{serial}/recording
Toggle recording on/off.

Response: 200

### PATCH /v1/camera/{serial}/recording
Toggle pause/resume while recording.

Response: 200

### GET /v1/camera/{serial}/recording
Get current recording state.

Response: 200
```json
{
    "recording": null
}
```