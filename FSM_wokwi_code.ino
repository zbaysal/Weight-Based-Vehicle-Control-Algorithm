#include "HX711.h"
HX711 scale;

const int DOUT_PIN = 2;
const int SCK_PIN = 4;

int action_value = -1; // Değişken, entegrede grup arkadaşımın yazdığı foksiyondan değer alıyor.
//Ama bu kodda manuel olarak değiştirilecek.
// 1  => Sipariş aracın üzerine bırakılmış.
//-1  => Boşlar aracın üzerine bırakılmış.
// 2 => Sipariş teslim edilmiş.
//-2 => Boşlar araç üzerinden alınmış.

int order = 5; // Değişken, entegrede grup arkadaşımın yazdığı foksiyondan değer alıyor.
//Ama bu kodda manuel olarak değiştirilecek.
//  3 = sipariş alınacak. 
//  4 = sipariş teslim edilecek.
//  5 = masadan boşlar alınacak.

int weightTimeout = 0;

enum weightState {
  ARAC_DOLU,
  EVE_GIDIS,
  ARAC_BOS,
  BOSLAR_ARACA_YUKLENDI,
  BOSLAR_ARACTAN_ALINDI,
  SIPARIS_ARACA_YUKLENDI,
  SIPARIS_ARACTAN_ALINDI,
  SIFIR_HALI,
  MASAYA_GIDIS
};

weightState wmState = SIFIR_HALI;

void setup() {

  Serial.begin(9600);
  scale.begin(DOUT_PIN,SCK_PIN);
  scale.set_scale(420.0);
  

}

void loop() {
  WeightMovement();

}
void WeightMovement(){
  float weight = scale.get_units(10);
  switch (wmState) {

    case SIFIR_HALI:
    Serial.print(" STATE = SIFIR HALI ");
    weight = scale.get_units(10); // 10 Ölçüm ortalaması ile ağırlığı hesaplayacak.
    Serial.print("Agirlik = ");
    Serial.println(weight);

      if (weight > 0.5 ){ //İlk karar merkezi ve state değiştirmesi.
        wmState = ARAC_DOLU;
      }
      if (weight <= 0.5 ){
        wmState = ARAC_BOS;
      }
    break;

    case EVE_GIDIS:
      Serial.print(" STATE- EVE_GIDIS ");
      Serial.print(" EVE ULASILDI.- Algoritma bastan baslatilacak. ");
      wmState = SIFIR_HALI;
      break;

    case MASAYA_GIDIS:
      Serial.print(" STATE- MASAYA_GIDIS ");
      Serial.print(" Masaya gidiliyor..... "); //Entegre proje kodunda bu kısımlarda diğer fonksiyonlar yer alıyor.
      wmState = ARAC_DOLU;
      break;


    case ARAC_DOLU:
      Serial.print(" STATE = ARAC DOLU ");
      delay(20);
      weight = scale.get_units(10);
      Serial.print(" Yeniden Olcum Gerceklestirildi. ");
      Serial.print(" Agirlik = ");
      Serial.println(weight);
      if (weight> 0.5) {
        delay(500);
        while (weight > 0.5 && weightTimeout < 20) { // Eğer hala araçta o ağırlık varsa araç yüklüdür. AĞIRLIK KALKTI MI ->(HAYIR)-> SİPARİŞİN TESLİM EDİLMESİNİ BEKLE
          delay(200);
          Serial.println(weightTimeout);
          Serial.print(" Agirlik Kontrol Ediliyor.. ");
          weight = scale.get_units(10); //Yeniden ölçüm gerçekleştirildi.
          weightTimeout++;
        }
        if (weight > 0.5) { 
          Serial.println(" Aractaki agirlik hareket ettirilmemistir. State degistirilmeyecektir. ");
          weightTimeout = 0;
          wmState = ARAC_DOLU;
      } 
      else {
          weightTimeout = 0;
          if (order == 4 && action_value == 2) {
              wmState = SIPARIS_ARACTAN_ALINDI;
          }
          else if (order == 5 && action_value == -2) {
              wmState = BOSLAR_ARACTAN_ALINDI;
          }
          else {
            Serial.println(" Order ve action_value degerleri saglanmadigindan state degistirilemeyecektir. ");
            Serial.println(" Lütfen order ve action_value degerlerini degistirerek bir daha deneyiniz. ");
            wmState = ARAC_DOLU;

          }
      }
      if(weight<0.5) {
        if (order ==4 && action_value == 2){
          wmState = SIPARIS_ARACTAN_ALINDI;
        }
        else if (order ==5  && action_value == -2){
          wmState = BOSLAR_ARACTAN_ALINDI;
        }

      }
      break;
    
    case SIPARIS_ARACTAN_ALINDI:
      Serial.print(" STATE = SIPARIS ARACTAN ALINDI ");
      delay(1000);
      TurningHalf(); // Grup arkadaşlarımdan birinin yazmış olduğu fonksiyon. Entegre kodu ayrı tutarak yüklediğimi belli etmek amacıyla temsili olarak bırakıyorum.
      wmState = EVE_GIDIS;
      break;


    case BOSLAR_ARACTAN_ALINDI:
        Serial.print(" STATE = BOSLAR ARACTAN ALINDI ");
        delay(100);
        wmState = SIFIR_HALI;
        break;

      case ARAC_BOS:
        Serial.print(" STATE = ARAC BOS ");
        scale.tare();
        delay(20); 
        weight = scale.get_units(10); //Yeniden ölçüm gerçekleştirildi.
        Serial.print(" Yeniden Olcum Gerceklestirildi. ");
        Serial.print(" Agirlik = ");
        Serial.println(weight);

        if (weight> 0.5) {
          if (order ==4 && action_value==1){
            wmState = SIPARIS_ARACA_YUKLENDI;
          }
          else if (order ==5 && action_value ==-1){
            wmState = BOSLAR_ARACA_YUKLENDI;
          }
          else {
            Serial.println(" Order ve action_value degerleri saglanmadigindan state degistirilemeyecektir. ");
            Serial.println(" Lütfen order ve action_value degerlerini degistirerek bir daha deneyiniz. ");
            wmState = ARAC_BOS;

          }
        }
        else {
          delay(500); 
          while (weight < 0.5 && weightTimeout < 20) { //Eğer araçta hala o ağırlık yoksa araç yüksüzdür. AĞIRLIK BİNDİ Mİ -> // (HAYIR)-> SİPARİŞİN HAZIRLANMASINI BEKLE
          weight = scale.get_units(10); //Yeniden ölçüm gerçekleştirildi.
          Serial.println(weightTimeout);
          Serial.print(" Yeniden Ölçülüyor.. ");
          delay(200);
          weightTimeout++;
        }
          if (weight < 0.5) { 
            Serial.println(" Araca agirlik verilmemistir. State degistirilmeyecektir. ");
            weightTimeout = 0;
            wmState = ARAC_BOS;
          } 
        }
        break;

      case SIPARIS_ARACA_YUKLENDI:
        Serial.print(" STATE =SIPARIS ARACA YUKLENDI ");
        delay(150);
        wmState =   MASAYA_GIDIS;
        break;

      case BOSLAR_ARACA_YUKLENDI:
        Serial.print(" STATE = BOSLAR ARACA YUKLENDI ");
        delay(100);
        TurningHalf();
        wmState = EVE_GIDIS;
        break;

        

      }

  }
}

void TurningHalf(){
  Serial.print(" Turning_Half fonksiyonu calisiyor.... "); //Entegre koddaki fonksiyon bloğu değil, simülasyonda görünmesi için yazıldı.
}

