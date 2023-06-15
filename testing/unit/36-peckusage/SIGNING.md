mic.pem and vendor_secp384r1.crt
--------------------------------

These files come from Minerva Highway tests data.
Minerva is at: https://github.com/AnimaGUS-minerva/highway.git

The mic.pem file comes from spec/files/cert/00-D0-E5-F2-00-02/device.crt
The vendor\_secp384r1.crt file comes from  spec/files/cert/vendor\_secp384r1.crt

The command: rake highway:h1\_bootstrap\_ca RESIGN=true
can be used to update the CA certificate.

The command: rake highway:signmic EUI64=00-D0-E5-F2-00-02
can be used to update the IDevID.
