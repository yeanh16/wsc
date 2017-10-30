void dostuff(int sock);
void *handle(void *param);
void error(char *msg);
void LoadCertificates(SSL_CTX* ctx, char* CertFile, char* KeyFile);
SSL_CTX* InitServerCTX(void);
void write_new(int sock, char *msg);
void ShutdownSSL(SSL *ssl);
