echo "### Metin2 DB Builder ###"
echo "### Compilando a DB... ###"
gmake -j20 > ../DB_BUILD_LOG.txt 2> ../DB_ERROR_LOG.txt
echo "### Finalizado ###"