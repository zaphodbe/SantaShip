TEMPLATE = subdirs

SantaShip.file = SantaShipApp/SantaShip.pro

SUBDIRS = \
        SMTPEmail \
        SantaShip

CONFIG += ordered qt

SantaShip.depends = SMTPEmail
