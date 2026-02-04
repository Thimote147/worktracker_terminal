#!/bin/bash

# Demande des informations à l'utilisateur
read -p "Heure d'arrivée (HH:MM) : " arrivee
read -p "Début de pause midi (HH:MM) : " pause_debut
read -p "Fin de pause midi (HH:MM) : " pause_fin

# Conversion des heures en secondes depuis le début de la journée
# On utilise la date du jour pour faciliter le calcul
start_sec=$(date -d "$arrivee" +%s)
lunch_start_sec=$(date -d "$pause_debut" +%s)
lunch_end_sec=$(date -d "$pause_fin" +%s)

# Calcul de la durée de la pause en secondes
pause_duree=$((lunch_end_sec - lunch_start_sec))

# Durée de travail souhaitée (8h = 8 * 3600 secondes)
travail_sec=$((8 * 3600))

# Calcul de l'heure de départ : Arrivée + Travail + Pause
depart_sec=$((start_sec + travail_sec + pause_duree))

# Affichage du résultat
heure_depart=$(date -d "@$depart_sec" +"%H:%M")

echo "----------------------------------------"
echo "Temps de pause : $((pause_duree / 60)) minutes"
echo "Pour prester 8h, tu dois partir à : $heure_depart"
echo "----------------------------------------"
