# Il progetto
Alla base del progetto c'è il microcontrollore **STM32F103C6T6**. Ho usato poi un servo **MGR996R** collegato alle pastiglie del freno, l'encoder magnetico **AS5600** per la misura della velocità, un sensore hall **3144EUA** per la misura dell'angolo del freno e uno schermo OLED con controllore SH1106. Il firmware è stato tutto scritto da me, compreso il [driver per lo schermo OLED](https://github.com/Kikkiu17/STM32_SH1106), tranne il driver per l'encoder AS5600. Tutti i pezzi sono stati stampati in 3D in PETG, PLA e TPU con una stampante Bambu Lab A1 Mini.

<p float="left">
  <img src="https://github.com/Kikkiu17/BicycleABS/blob/main/Immagini/3d_servo_sensore.png" width="33%"/>
  <img src="https://github.com/Kikkiu17/BicycleABS/blob/main/Immagini/3d_encoder_freno.png" width="33%"/>
  <img src="https://github.com/Kikkiu17/BicycleABS/blob/main/Immagini/3d_batteria_schermo.png" width="33%"/>
</p>

In base all'angolo della maniglia del freno, il servo applica più o meno pressione alle pastiglie del freno, rallentando quindi la ruota e la bici. Quando la ruota si blocca, ad esempio se si frena tanto sullo sterrato, il microcontrollore rileva questa condizione e rilascia le pastiglie, in modo da sbloccare la ruota. Poi, riapplica pressione in modo da ricominciare a frenare e nel caso il ciclo continua.

Lo schermo OLED posto sul lato destro manubrio permette di vedere la velocità attuale della bici, i chilometri percorsi, il livello della batteria, lo stato delle luci (spente/accese/lampeggianti - la luce frontale, a cui ho tolto la batteria, era compresa con la bici, mentre la luce posteriore è fatta con due LED rossi) e se l'ABS si è attivato. Sul lato sinistro del manubrio invece è posto un pulsante che può essere usato per diverse cose. Se si fa una pressione breve si cambia la modalità delle luci (spente/accese/lampeggianti) in maniera ciclica. Se si fa una pressione lunga si entra nel menù, dal quale si possono modificare varie impostazioni (come l'angolo del servo, che quindi corrisponde alla pressione massima del freno, oppure la luminosità dello schermo), visualizzare diversi parametri di debug (come variabili interne dei TIMER del microcontrollore) e non (come la tensione della batteria).

<img src="https://github.com/Kikkiu17/BicycleABS/blob/main/Immagini/menu.gif" width="500"/>

# Costruzione - Hardware
Lo sviluppo dell'hardware è stato molto impegnativo. Il mio piano era modulare: sarei partito con la cosa più semplice, ovvero il supporto del servo, per fare un sistema **_brake by wire_**. In caso di successo, avrei continuato con il sensore dell'angolo della maniglia del freno (che d'ora in poi chiameremo **sensore freno**). In caso di successo, avrei continuato infine con il sensore di velocità della ruota. In questo modo avrei sprecato meno tempo possibile in caso di fallimento di qualche pezzo.

## Supporto e bracci servo
Per avvitare il supporto alla bici, ho sfruttato due buchi sul telaio apparentemente inutil in una posizione convenientemente vicina alle pastiglie del freno posteriore. Ho creato un supporto molto resistente e molto difficile da piegare in **PETG**, plastica adatta all'uso esterno (resistente ai raggi UV e al calore). Dopo ho realizzato dei bracci corti (in modo da massimizzare la coppia), da avvitare ai bracci del freno esistenti, **_print in place_**, ovvero che non necessitano di assemblaggio dopo la stampa (questo perché sono presenti due cerniere, difficili da fare resistenti e adatte all'assemblaggio). In questo modo il servo rimane ben saldo al telaio della bici e può fare tutta la forza che vuole.

<img src="https://github.com/Kikkiu17/BicycleABS/blob/main/Immagini/servo.jpg" width="500"/>
<img src="https://github.com/Kikkiu17/BicycleABS/blob/main/Immagini/freno.gif" width="500"/>

## Sensore freno
Il sensore consiste in un magnete che ruota e una scheda con un chip che registra l'angolo del campo magnetico prodotto dal sensore. Non c'è però nessun punto comodo in cui montare né la parte fissa (il chip), né quella mobile (il magnete). Per il magnete quindi ho creato una sorta di morsa con dei gommini morbidi (quelli blu nella GIF) avvitata attorno alla maniglia del freno. Questa morsa ha un prolungamento su cui è situato il magnete, che si trova proprio sull'asse di rotazione della maniglia. Per il chip ho creato un supporto da avvitare al manubrio e che si estende fino a sopra il magnete, in modo da poter registrare il suo angolo. In base a quest'angolo, poi, determino l'angolo della maniglia e quindi quanto sto frenando. Dopo alcuni calcoli e controlli quest'angolo viene trasformato nell'angolo servo che viene direttamente inviato a quest'ultimo.

<img src="https://github.com/Kikkiu17/BicycleABS/blob/main/Immagini/sensore_freno.gif" width="500"/>

## Sensore di velocità
Il sensore di velocità comprende un semplice sensore **_hall_** (che rileva la presenza di un magnete o meno) e 30 magneti montati su un disco, a sua volta montato sulla ruota posteriore. Misurando quanti magneti vengono rilevati al secondo, si può determinare la velocità della ruota. Anche qui il problema è che non c'è nessun punto di montaggio comodo. Ho dovuto quindi creare il disco arancione in due pezzi in modo da poterlo mettere sull'asse di rotazione senza dover smontare l'intera ruota (il che significherebbe smontare anche il cambio... cosa che non avevo voglia di fare decine di volte) e per tenerlo fisso ad essa ho creato il pezzo nero che va ad avvitarsi dietro. In questo modo il pezzo arancione e il pezzo nero, avvitandosi, vanno a stringersi sui raggi del cerchio della ruota, effettivamente bloccandoli ad essa.

Il supporto del sensore **_hall_** è messo sull'asse della ruota e tenuto fermo con il dado originale che tiene fissa la ruota al telaio.

<img src="https://github.com/Kikkiu17/BicycleABS/blob/main/Immagini/sensore_velocita.jpg" width="500"/>

## Batteria
Ho anche creato un contenitore per una batteria LiPo 2S (7.4 V) che contiene anche un modulo per la ricarica della stessa. Ho voluto creare questo contenitore in modo da poter estrarre la batteria dalla bici in modo facile e veloce, così che posso portarla comodamente con me quando devo ricaricarla.

<img src="https://github.com/Kikkiu17/BicycleABS/blob/main/Immagini/batteria.gif" width="500"/>

# Circuito elettronico
## Primo prototipo
Ho costruito il primo prototipo di circuito su una scheda millefori contemporaneamente allo sviluppo dei pezzi stampati in 3D. Dopo aver installato tutte le componenti e aver scritto parte del codice, ho realizzato che una scheda del genere non era adatta. Questo perché facendo pressione con il coperchio della scatola della scheda, oppure con qualche vibrazione (ad esempio passando dalla strada al marciapiede), il programma si bloccava e il servo rimaneva nella posizione di massima pressione. Dopo aver cercato per diverso tempo la causa del problema e non avendola trovata, ho deciso di abbandonare lo sviluppo su questa scheda. Questa però non è stata inutile perché mi ha permesso di testare le componenti e il programma, quindi sono passato allo sviluppo di una PCB (era solo un **_proof of concept_**).

<img src="https://github.com/Kikkiu17/BicycleABS/blob/main/Immagini/scheda_millefori.jpg" width="500"/>

## PCB
Ho creato quindi la PCB con tutti i vari MOSFET per poter controllare la luce frontale e posteriore (luce posteriore che si accende quando si frena, mentre rimane spenta quando non si frena), il servo e il sensore di velocità. Ho messo poi due connettori per collegare lo schermo OLED e il sensore freno via I²C al microcontrollore. Ho lasciato anche dello spazio per il convertitore DC-DC dalla tensione della batteria a 6 V, ovvero la tensione per il servo, aggiungendo poi un altro regolatore di tensione per trasformare questi 6 V in 3.3 V per il microcontrollore.

Ho fatto qualche errore, ovvero che le impronte dei MOSFET non erano corrette, quindi quando ho assemblato la PCB ho dovuto ruotare questi componenti di 90°... però funziona.

<img src="https://github.com/Kikkiu17/BicycleABS/blob/main/Immagini/schema.png" width="500"/>
<img src="https://github.com/Kikkiu17/BicycleABS/blob/main/Immagini/pcb.png" width="500"/>
<img src="https://github.com/Kikkiu17/BicycleABS/blob/main/Immagini/pcb_assemblata.jpg" width="500"/>

# Firmware
## Calcolo velocità
L'output del sensore hall sulla ruota è un'onda quadra con un certo periodo. Questo periodo idealmente dovrebbe essere in microsecondi, così la conversione in secondi sarebbe molto semplice. C'è una limitazione: i timer dell'STM32F103 sono a 16-bit, quindi il valore massimo è 65535 - se questi fossero microsecondi, il periodo massimo registrabile sarebbe di circa 65 millisecondi. Facendo $f_{mag}=\frac{1}{T_{hall}}=\frac{1}{0.065}=15.38\text{ Hz}$ otteniamo la frequenza di rilevamento dei magneti dal sensore. Dato che abbiamo 30 magneti: $f_{ruota}=\frac{f_{mag}}{30}=\frac{15.38}{30}=0.51\text{ Hz}$.

Questo è un moto circolare, quindi per calcolare la velocità tangenziale bisogna fare: $v_{ruota}=\frac{r\Delta\theta}{\Delta t}=r\Delta\theta f_{ruota}$. Visto che abbiamo il periodo di un'intera rotazione, consideriamo che l'arco sia la circonferenza della ruota: $\Delta\theta=\frac{arco}{raggio}=\frac{2\pi r}{r}=2\pi$, quindi la formula iniziale diventa: $v_{ruota}=2\pi rf_{ruota}$.

Ora possiamo finalmente vedere la velocità minima registrabile, col periodo massimo di 65 millisecondi, ovvero 0.51 Hz. Considerando il raggio della ruota $r=0.34\text{ m}$: $v_{ruota}=2\pi (0.34\text{ m})(0.51\text{ Hz})=1.08\text{ m/s}$ ovvero 3.9 km/h. Questa è una velocità minima un po' troppo alta e quindi sarebbe difficile rilevare quando la ruota è effettivamente ferma.

Dobbiamo quindi aumentare il periodo massimo registrabile. Fino ad ora, abbiamo considerato che un "tick" equivalga a un microsecondo, ma ora dobbiamo farlo valere di più: ad esempio, se 1 tick = 2 microsecondi, allora il periodo massimo registrabile sarebbe il doppio, ovvero 130 millisecondi e utilizzando la formula di prima la velocità minima sarebbe di 1.97 km/h che è già più accettabile. Per fare questo calcolo nel software, dobbiamo considerare la frequenza del clock del timer, che è uguale per tutte le periferiche `APB1` e in questo caso è di `64 MHz`. Il valore di ogni tick è dato dalla frequenza di clock del timer diviso il "prescaler" `PSC`. Quindi, se il prescaler fosse di 128, avremmo $f_{TIM}=\frac{64000000}{128}=500\text{ kHz}$ quindi il tick sarebbe di 2 microsecondi. Ora dividiamo la frequenza di ogni tick per il numero di tick registrati ($k_{hall}$) e otteniamo la frequenza dei magneti.

Quindi:

$f_{TIM}=\frac{f_{APB1}}{PSC}$

$f_{mag}=\frac{f_{TIM}}{k_{hall}}$

$f_{ruota}=\frac{f_{mag}}{30}=\frac{\frac{f_{TIM}}{k_{hall}}}{30}=\frac{\frac{\frac{f_{APB1}}{PSC}}{k_{hall}}}{30}$ -> questa è infine, facendo qualche semplificazione, la formula usata nel programma, in C: `freq = ((float)(apb1_timer_clock / 10 / ((TIM2->PSC + 1) / 1000)) / (float)period) / 30.0` da cui si ricava la velocità.
## Rilevamento blocco
Ci sono due timer in azione: il primo è quello che è stato usato finora per calcolare la velocità della ruota. Il secondo viene usato per il rilevamento del blocco di essa. Il secondo timer ha una frequenza di 20 Hz, quindi viene controllato se la ruota è bloccata ogni 50 ms. Ogni 50 ms, quindi, il secondo timer controlla il periodo del primo timer: se questo è troppo alto, ovvero più alto del periodo del secondo timer, significa che la ruota ha rallentato molto, quindi si può considerare che stia scivolando. Inoltre, il primo timer, ogni volta che il sensore hall rileva un magnete, resetta il valore del secondo timer, quindi il rilevamento del blocco è migliore. L'ABS, quindi, se la ruota scivola, si attiva.
