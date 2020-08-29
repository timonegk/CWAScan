from extensions import db

class Scan(db.Model):
    __tablename__ = 'scans'

    id = db.Column(db.Integer, primary_key=True)
    time = db.Column(db.DateTime, nullable=False)
    latitude = db.Column(db.Float, nullable=False)
    longitude = db.Column(db.Float, nullable=False)

class Address(db.Model):
    __tablename__ = 'addresses'
    scan_id = db.Column(db.Integer, db.ForeignKey('scans.id'), primary_key=True)
    scan = db.relationship('Scan', backref='addresses')
    address = db.Column(db.String, primary_key=True)
