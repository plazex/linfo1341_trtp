# Projet reseau: implementation du protocole trtp
## Comment compiler
1. `cd <projet>`
2. `make clean`
3. `make`

## Comment executer
### Sender
##### Paramètres
L'ordre d'occurence des paramètres n'est pas important à l'exception du port qui doit venir après l'adresse.
1. -f <file_name> : fichier à envoyer. Possible d'utiliser '<' pour rediriger le contenu d'un fichier.
2. -s <stat_file_name> : nom du fichier d'enregistrement des statistiques (optionel).
3. -l <log_file_name> : nom du fichier d'enregistrement des logs (optionel).
4. -c : active l'utilisation des paquets de type FEC (optionel)
5. -o <percentage> : pourcentage de paquets perdus (s'applique de manière aléatoire).
6. -t <percentage> : pourcentage de paquets trunqués (s'applique de manière aléatoire).
7. <receiver_address> <receiver_port> : addresse et port du recepteur (requis dans cet ordre).
##### Example
`./sender -f to_send.txt -s stat_send.txt -l send_log.txt -o 10 -t 10 -c ::1 4567`

### Receiver
##### Paramètres
L'ordre d'occurence des paramètres n'est pas important à l'exception du port qui doit venir après l'adresse.
1. -f <file_name> : nom du fichier d'enregistrement.
2. -s <stat_file_name> : nom du fichier d'enregistrement des statistiques (optionel).
3. -l <log_file_name> : nom du fichier d'enregistrement des logs (optionel).
5. -o <percentage> : pourcentage de paquets perdus (s'applique de manière aléatoire).
6. -t <percentage> : pourcentage de paquets trunqués (s'applique de manière aléatoire).
7. <receiver_address> <receiver_port> : addresse et port d'ecoute (requis dans cet ordre).
##### Example
`./receiver -f to_rcv.txt -s stat_recv.txt -l recv_log.txt ::1 4567`