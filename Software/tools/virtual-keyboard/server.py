from flask import Flask, request, session, make_response, render_template, redirect
from time import time
import socket


app = Flask(__name__)

# list to keep track of the received log
logs = []

# if POST request:
#   add the body to logs
# otherwise:
#   return virtual keybord application
@app.route("/", methods = ["GET", "POST"])
def index():
    if request.method == "POST":
        msg = request.get_data().decode()

        later = []
        while logs and int(logs[-1].split()[0]) > int(msg.split()[0]):
            later.append(logs.pop())
            
        logs.append(msg)
        logs.extend(later[::-1])
            
        try:
            millis, *_ = msg.split()
            print(f"Delay: {time()*1000 - float(millis)}")
        except:
            print("ERROR: Message is malformatted!")
            
        return "successful"
    else:
        ipaddr = socket.gethostbyname(socket.gethostname())
        return render_template("index.html", ipaddr = ipaddr)

        
# clear logs
@app.route("/clear", methods = ["GET"])
def clear():
    print(f"Cleared {len(logs)} entries.")
    logs.clear()
    return "log cleared"


# return logs
@app.route("/logs", methods = ["GET"])
def get_logs():
    return "<br>".join(logs)

if __name__ == '__main__':
    app.run(host="0.0.0.0", port=8080, debug=True)
