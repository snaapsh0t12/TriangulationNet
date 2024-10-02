"""
This is the file for the client

It could be rewritten in bash, so it could be changed
"""

import requests

url = 'http://127.0.0.1:5000/register'
myobj = {'id': '2'}

x = requests.post(url, json = myobj)

print(x.text)