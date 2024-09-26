"""
This is the server file for the central server

It receives pings, and uses it with the location of nodes to calculate the location of the target.

This is the server file, its a basic Flask server which creates an image

Functions:

    - Register | add a new or rebooted node to the network
        - Pings the dashboard and tells the user to add coordinates for the node. After coordinates are received adds node to the cache (timestamp 0) and nodes database (dont want nodes without coordinates being added to the math)

    - Target ping | a noe tells the server it can ping the target
        - Takes the node id and the target_address from the node, and writes with the time to the cache

    - Config version | node requests the current config version
        - Returns the sha256 hash of the config file

    - Config | node requests the config file
        - Returns a file.read() of the config file

    - Config change | user changes the config file form the dashboard
        - Takes the new config parameters and writes them to the config file

    - Database change | user wants to change the coordinates of a node (submits the coordinates and ids for all node because I hate frontend)
        - Takes the ids and coordinates for the nodes and writes them to the database

"""

from flask import Flask, request, render_template, jsonify
import hashlib

app = Flask(__name__)

global toAdd
toAdd=[]

@app.route('/', methods=['GET'])
def index():
    # The index webpage with the real time map of the network
    number=1
    
    return render_template('index.html', number=number)

@app.route('/register', methods=['POST'])
def register():
    # For the nodes only, the form to actually register nodes is at /register_final
    global toAdd
    id = request.args.get('id')
    toAdd.append(id)

    # Ping the dashboard with the id
    
    return 

@app.route('/register_final', methods=['POST'])
def register_final():
    # For the user submitted form
    id = request.args.get('id')
    x = request.args.get('x')
    y = request.args.get('y')

    # Write to the database
    
    return 

@app.route('/ping', methods=['POST'])
def ping():
    # Node can ping the target
    id = request.args.get('id')

    # Write to the cache
    
    return 

@app.route('/config_version', methods=['GET'])
def config_version():
    # Returns the sha256sum of the config
    
    return hashlib.sha256(open('config', 'rb').read()).hexdigest()

@app.route('/config', methods=['GET'])
def config():
    # Returns the config

    return open('config').read()

@app.route('/config_change', methods=['POST'])
def config_change():
    # Changes the config

    # When the user clicks a button the current config is loaded into a form, and the user can edit stuff and hit submit, which sends the new config to this route

    target_address = request.args.get('target_address')
    le = request.args.get('le')
    config_update_wait = request.args.get('config_update_wait')
    clock_delay = request.args.get('clock_delay')

    # Load these new settings into the config

    return 

@app.route('/database_change', methods=['POST'])
def database_change():
    # Changes the config

    # When the user clicks a button the current database is loaded into a form, and the user can edit stuff and hit submit, which sends the new database to this route

    # Read the submitted list of nodes and coordinates

    # Load the new database into the database

    return 

global reposnes
reposnes=0
@app.route('/check', methods=['GET'])
def check():

    global reposnes

    response = {}

    if reposnes%1==0:
        print("Sent  check response")
        response = {"title": "My Title"+str(reposnes)}

    reposnes+=1
    
    return jsonify(response)

@app.route('/submit', methods=['POST'])
def submit():

    print(request.get_json())


    return "woah"

@app.route('/nt', methods=['GET'])
def notify():
    # The index webpage with the real time map of the network
    
    return render_template('notify.html')



if __name__ == '__main__':
    app.run(debug=True)