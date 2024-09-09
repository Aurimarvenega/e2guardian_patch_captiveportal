# E2Guardian - Integração de Autenticação com Captive Portal
Patch para permitir o Proxy E2Guardian identificar o usuário logado através do Captive Portal no pfSense


!! Importante: Este procedimento foi homologado para pfSense CE na versão 2.7.2  
Última atualização: 01/05/2024   
Versão: 1.3 - atualização URL do repositório extra oficial de plugins e adicionado seção de troubleshooting ao final deste documento.  
Versão: 1.3 - atualização do procedimento para pfSense 2.7.2.  
Versão: 1.2 - correções para detectar logout dos usuários e limpar cache junto ao captive portal.  
Versão: 1.1 - publicação inicial.  
Responsável: Luciano Rodrigues - luciano@citrait.com.br  

## Instalação:



### 1- Ajustar hostname e nome de domínio do firewall de acordo com o seu ambiente:
1.1- A configuração é feita pelo menu System -> General Setup.

### 2- Criar o arquivo de configuração externa do serviço DNS:  
2.1- Criar o arquivo /var/unbound/dnsauth.conf vazio com o comando abaixo (executar através do menu Diagnostics -> Command Prompt):  
```
echo> /var/unbound/dnsauth.conf
```  
  
### 3- Configure o serviço de DNS do pfSense para incluir o arquivo dnsauth.conf.  
3.1- Acesse o menu Services -> DNS Resolver. Clique em Display Custom Options ao final da página e inclua a linha seguinte no campo de texto:  
```
include: /var/unbound/dnsauth.conf
```  
3.2- Importante: Você não deve habilitar o encaminhamento DNS (Query Forwarding), pois não irá funcionar com esta configuração.  


### 4- Realize a instalação do E2Guardian:  
4.1- Vá no menu System -> Package Manager -> Available Packages e instale o pacote System_Patches.  
4.2- Acesse a página abaixo e copie todo o código:  
```
https://raw.githubusercontent.com/marcelloc/Unofficial-pfSense-packages/master/27_unofficial_packages_list.patch
```  
4.3- Acesse o menu System -> Patches. Clique em Add New Patch.  
4.4- Cole o código no campo Patch Contents.  
4.5- Em Description preencha com: Unnoficial_packages.  
4.6- No campo Path Strip Count defina com valor 1 e salve o patch (botão save ao final da página).  
4.7- Clique em apply no patch recem registrado.  
4.8- No menu Diagnostics -> Command Prompt execute o comando abaixo:  
```
fetch -q -o /usr/local/etc/pkg/repos/Unofficial.conf 
https://raw.githubusercontent.com/marcelloc/Unofficial-pfSense-packages/master/Unofficial_25.conf
```
4.9- Ainda no menu Diagnostics -> Command Prompt execute o comando abaixo:
```
pkg update
```
4.10- Acesse o menu System -> Package Manager -> Available Packages e instale o pacote E2Guardian.  
4.11- Execute os comandos abaixo para resolver os problemas de DLL faltando no E2Guardian:  
```
ln -s /usr/lib/libssl.so.30 /lib/libssl.so.111
ln -s /lib/libcrypto.so.30 /lib/libcrypto.so.111
```  

### 5- Realize a configuração do captive portal (adicione uma zona ao captive portal).  
5.1- Acesse Zones -> Captive Portal e clique em Add.  
5.2- Dê um nome e uma descrição para a zona e clique em Add & Continue.  
5.3- Clique em Enable, selecione a interface (LAN).  
5.4- No campo Authentication Server selecione Local database.  
5.5- Clique em Save ao final da página.  

### 6- Instale o patch do captive portal.  
6.1- Acesse a página abaixo e copie todo o código:  
```
https://raw.githubusercontent.com/CitraIT/e2guardian_patch_captiveportal/main/patches/captiveportal.patch
```
6.2- Acesse o menu System -> Patches e clique em Add Patch.  
6.3- Cole o código no campo Patch Contents.  
6.4- Em Description preencha com: CitraIT_Patch_CaptivePortal_Index_PHP.  
6.5- No campo Path Strip Count defina com valor 1 e salve o patch (botão save ao final da página).  
6.6- Clique em Save ao final da página.  
6.7- Clique em apply no patch recem registrado.  

### 7- Ajustar o plugin de autenticação do E2Guardian.  
7.1- Acesse o menu Diagnostics -> Edit File.  
7.2- Insira no caminho do arquivo o texto abaixo e clique em Load:  
```
/usr/local/etc/e2guardian/authplugins/dnsauth.conf
```
7.3- Apague todo o texto e cole o texto abaixo:  
```
# IP/DNS-based auth plugin
#
# Obtains user and group from domain entry maintained by separate authentication# program.

plugname = 'dnsauth'

# Base domain
basedomain = "citrait.local"

# Authentication URL
authurl = "https://192.168.1.1:8002/?redirurl"

# Prefix for auth URLs
prefix_auth = "https://192.168.1.1:8002/"

# Redirect to auth (i.e. log-in)
#  yes - redirects to authurl to login
#  no - drops through to next auth plugin
redirect_to_auth = "yes"
```
7.4- Substitua citrait.local pelo nome de domínio do firewall (ex.: empresa.corp).  
7.5- Substitua a authurl pelo endereço no qual o pfSense te redireciona para o Captive Portal.  
7.6- Em prefix_auth ajuste conforme a variável authurl, mas terminando na barra após a porta.  
7.7- Clique em Save para salvar o arquivo.  

### 8- Crie um usuário de teste.  
8.1- Acesse o menu System -> User Manager e clique em Add.  
8.2- Preencha o nome e senha do usuário e clique em Save.  
8.3- Edite o usuário criado acima e atribua a ele a permissão "User - Services: Captive Portal login".  

### 9- Criar uma CA (Autoridade Certificadora) para usar na interceptação SSL/HTTPS do E2Guardian.   
9.1- Acesse o menu System -> Cert. Manager, clique em Add.  
9.2- Dê um nome descritivo para a CA (ex.: CA-E2GUARDIAN).  
9.3- Marque a caixa "Trust Store".  
9.4- Em "Common Name" preencha com um nome significativo (ex.: usar o mesmo da descrição).  
9.5- Preencha as informações organizacionais (country code, state, city...) e clique em Save.


### 10- Baixar a biblioteca de rotinas da integração.  
10.1- No menu Diagnostics -> Command Prompt execute o comando abaixo:  
```
fetch -q -o /etc/inc/captive2guardian.inc https://raw.githubusercontent.com/CitraIT/e2guardian_patch_captiveportal/main/etc/inc/captive2guardian.inc
```  


### 11- Aplicar o patch da biblioteca do captive portal:  
11.1- Acesse a página abaixo e copie todo o código:  
```
https://raw.githubusercontent.com/CitraIT/e2guardian_patch_captiveportal/main/patches/captiveportal_inc.patch
```  
11.2- Acesse o menu System -> Patches. Clique em Add New Patch.  
11.3- Cole o código no campo Patch Contents.  
11.4- Em Description preencha com: citrait_captive2guardian_inc.  
11.5- No campo Path Strip Count defina com valor 1 e salve o patch (botão save ao final da página).  
11.6- Clique em apply no patch recem registrado.  



### 12- Configurar o E2Guardian.  
12.1- Acesse o menu Services -> E2Guardian Proxy.  
12.2- Marque a caixa "Enable e2guardian".  
12.3- Selecione as interfaces LAN e Loopback.  
12.4- Marque a caixa "Transparent HTTP Proxy".  
12.5- Marque a opção "Bypass Proxy for Private Address Destination".  
12.6- Marque a opção "Enable SSL support".  
12.7- Selecione a CA que criou na etapa acima.  
12.8- Clique em Save.  

### 13- Habilitar a autenticação no E2Guardian.  
13.1- Acesso o menu Services -> E2Guardian Proxy -> Guia General.  
13.2- No campo "Auth Plugins" selecione apenas DNS.  
13.3- Clique em Save ao final da Página.  

### 14- Configurar um usuário e grupo de teste.  
14.1- Acesso o menu Services -> E2Guardian Proxy -> Guia Grups e clique em Add.  
14.2- Dê um nome e uma descrição para o grupo (ex.: ti / ti).  
14.3- Clique em Save ao final da página.  
14.4- Acesse a aba Users.  
14.5- No campo com o nome do grupo (ex.: ti) referente ao grupo ti, insira o login do usuário (ex.: luciano).  
14.6- Clique em Save ao final da página.  
14.7- Clique em Apply Changes (botão verde que aparece no topo após salvar a configuração).  



### 15- Testar  
15.1- Usando uma máquina que está dentro da zona do captive portal habilitado (LAN), navegue em algum site.  
15.2- Você deve ser redirecionado para a tela do captive portal.  
15.3- Após autenticar, deverá ser redirecionado para o site que tentou navegar.  
15.4- Acesse o menu Services -> E2Guardian Proxy.  
15.5- Clique na guia Real Time.  
15.6- Deverá aparecer nos logs o nome do usuário e o grupo que ele foi identificado semelhante a imagem abaixo:  

![image](https://user-images.githubusercontent.com/91758384/188039740-0e3cbd25-b9ae-4c37-8636-5a2e051f5ad5.png)


### 16- Resolução de Problemas  
16.1- Muito provavelmente você vai querer criar uma regra bloqueando o acesso direto da LAN a sites externos (tcp/udp) nas portas 80/443.  
16.2- Garanta que o firewall usa apenas ele mesmo como DNS (não deixar fazer sobrescrita de dns pela operadora).  
16.3- Navegadores hoje em dia usam o protocolo QUIC/udp por padrão, que é incompatível com o proxy transparente. Por algumas vezes as páginas irão dar erro e depois irão carregar. É uma limitação recente dos proxies.  
16.4- O E2Guardian já vem com a lista de categorias de sites Shallalist (em sua última atualização).  
16.5- Altere a porta da webgui do firewall e desabilite o redirect da webgui para evitar problemas.  
16.6- Use o captive portal com SSL habilitado e certificado válido (ainda que emitido pelo plugin acme/let's encrypt). Se não usar o ssl, edite o arquivo de autenticação do E2Guardian para refletir a url do captive portal sem HTTPS.    



