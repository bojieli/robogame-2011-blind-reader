while [ 1 ];
do
read barcode
curl "http://localhost/blind/updatebarcode.php?barcode=$barcode"
done
