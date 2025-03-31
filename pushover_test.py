import requests



def send_pushover_notification(message):
    user_key = "u4jn6g4g9rbsvnbsevfiymwk8uiuux"
    api_token = "aosytq27bi5x2nsk9gct1ofsozfwwm"
    
    url = "https://api.pushover.net/1/messages.json"
    data = {
        "token": api_token,
        "user": user_key,
        "message": message
    }

    response = requests.post(url, data=data)
    print(response.json())

send_pushover_notification("🚨 Smart House Alert! Motion detected in living room!")