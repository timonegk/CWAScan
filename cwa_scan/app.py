#!/usr/bin/env python3
from datetime import datetime
from flask import request, abort, Response

from extensions import app, db
from models import Scan, Address

with app.app_context():
    db.create_all()

@app.route('/', methods=['GET'])
def index():
    ensure_authenticated()
    return ''

@app.route('/', methods=['POST'])
def post():
    ensure_authenticated()
    if (not request.json or
            'time' not in request.json or
            'location' not in request.json or
            'data' not in request.json or
            len(request.json['location']) != 2):
        abort(400)

    scan = Scan(time=datetime.fromtimestamp(request.json['time']),
                latitude=request.json['location'][0],
                longitude=request.json['location'][1])
    db.session.add(scan)
    db.session.commit()

    for address in request.json['data']:
        a = Address(address=address, scan_id=scan.id)
        db.session.add(a)
    db.session.commit()

    return '', 201
    
def ensure_authenticated():
    if not request.authorization:
        abort(Response(status=401, headers={'WWW-Authenticate': 'Basic'}))

    username = request.authorization['username']
    password = request.authorization['password']

    if (username != app.config['BASIC_AUTH_USER'] or
            password != app.config['BASIC_AUTH_PASSWORD']):
        abort(Response(status=401, headers={'WWW-Authenticate': 'Basic'}))


if __name__ == '__main__':
    app.run()
