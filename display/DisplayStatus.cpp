#include "DisplayStatus.h"
#include <iomanip>
#include <sstream>
namespace ventilation::display {
DisplayStatus describe(const DisplaySession& s) {
    const auto& o=s.output();const auto f=s.feedback();
    DisplayStatus result;
    result.fan=o.fanRequested?L"BEKAPCSOLVA":L"KIKAPCSOLVA";
    result.dehumidifier=L"KIKAPCSOLVA";
    result.dehumidifierConfirmedRunning=o.dehumidifierRequested&&f.valid&&f.feedback==DehumidifierFeedback::Running;
    if(o.dehumidifierRequested) {
        if(result.dehumidifierConfirmedRunning) result.dehumidifier=L"MŰKÖDIK";
        else if(o.feedbackUnavailable) result.dehumidifier=L"NEM IGAZOLT";
        else if(f.feedback==DehumidifierFeedback::Starting||f.feedback==DehumidifierFeedback::Off) result.dehumidifier=L"MÉG NEM MEGY";
        else result.dehumidifier=L"NEM MŰKÖDIK";
    }
    if(o.sensorWarning&&s.mode()==Mode::Auto) {
        result.title=L"HIBA MIATT\nLEÁLLÍTVA";
        result.explanation=L"Nem kapunk megbízható érzékelőadatot.\nMindkét készüléket kikapcsoltuk.";result.tone=StatusTone::Error;
    } else if(s.mode()==Mode::Off) {
        result.title=L"KÉZZEL\nKIKAPCSOLVA";
        result.explanation=L"A szárítás le van tiltva.\nÚjraindításhoz válaszd az AUTO módot.";
    } else if(o.reasonCode==ReasonCode::TankFull) {
        result.title=L"TARTÁLY MEGTELT\nPÁRAMENTESÍTŐ KI";
        result.explanation=L"Ürítsd ki a víztartályt.\nA ventilátor ettől még működhet.";result.tone=StatusTone::Attention;
    } else if(o.reasonCode==ReasonCode::TankSensorFailure) {
        result.title=L"TARTÁLYJEL HIBÁS\nPÁRAMENTESÍTŐ KI";
        result.explanation=L"Ellenőrizd az úszót és a vezetékét.\nA páramentesítőt letiltottuk.";result.tone=StatusTone::Error;
    } else if(o.reasonCode==ReasonCode::AutoRestartUnverified) {
        result.title=L"PÁRAMENTESÍTŐ\nNEM INDÍTHATÓ";
        result.explanation=L"Előbb ellenőrizd: áramszünet után\nmagától újraindul-e a készülék.";result.tone=StatusTone::Attention;
    } else if(o.reasonCode==ReasonCode::DeviceFault) {
        result.title=L"PÁRAMENTESÍTŐ\nHIBÁT JELEZ";
        result.explanation=L"Kikapcsoltuk az áramellátását.\nEllenőrizd a készüléket.";result.tone=StatusTone::Error;
    } else if(o.fanRequested) {
        result.title=L"SZELLŐZTETÉS\nBEKAPCSOLVA";
        result.explanation=s.mode()==Mode::ManualVentilation?L"A ventilátort kézzel kapcsoltad be.":L"A kinti levegő most szárítja a pincét.";
        if(o.reasonCode==ReasonCode::MinimumOn) result.explanation=L"A ventilátort még járatjuk,\nhogy ne kapcsolgasson túl gyakran.";
        result.tone=StatusTone::Active;
    } else if(o.dehumidifierRequested) {
        if(o.reasonCode==ReasonCode::NoPowerDraw||f.feedback==DehumidifierFeedback::NoPowerDraw) {
            result.title=L"PÁRAMENTESÍTŐ\nNEM MŰKÖDIK";
            result.explanation=L"Bekapcsoltuk, de nincs érzékelt működés.\nEllenőrizd: tartály, készülék, tápellátás.";result.tone=StatusTone::Error;
        } else if(o.feedbackUnavailable) {
            result.title=L"PÁRAMENTESÍTŐ\nNINCS VISSZAJELZÉS";
            result.explanation=L"A bekapcsolást kértük, de a működését\nnem tudjuk ellenőrizni.";result.tone=StatusTone::Attention;
        } else if(result.dehumidifierConfirmedRunning) {
            result.title=L"PÁRAMENTESÍTŐ\nMŰKÖDIK";
            result.explanation=L"A készülék működését visszajelzés igazolja.";result.tone=StatusTone::Active;
            if(s.basement().isPlausible()&&s.basement().temperatureC<=s.configuration().minBasementTemperature)
                result.explanation=L"A pince túl hideg a szellőztetéshez.\nHelyette a páramentesítő dolgozik.";
        } else {
            result.title=L"PÁRAMENTESÍTŐ\nMÉG NEM INDULT EL";
            result.explanation=L"Áramellátása bekapcsolva.\nA kompresszor indulását ellenőrizzük.";result.tone=StatusTone::Attention;
        }
    } else if(o.state==ControllerState::WaitingMinimumOffTime) {
        std::wostringstream timer;const auto seconds=(o.remainingWaitMs+999)/1000;
        timer<<seconds/60<<L" perc "<<seconds%60<<L" mp";
        result.title=L"MOST EGYIK\nKÉSZÜLÉK SEM MEGY";
        result.explanation=std::wstring(o.reason[0]=='V'?L"Ventilátor":L"Páramentesítő")+L": még "+timer.str()+L".\nVédelmi szünet, utána újra döntünk.";
    } else if(o.reasonCode==ReasonCode::ClockFailure) {
        result.title=L"ÓRAHIBA MIATT\nLEÁLLÍTVA";result.explanation=L"Mindkét készülék kikapcsolva.\nIndítsd újra a vezérlőt.";result.tone=StatusTone::Error;
    } else {
        result.title=L"MOST NEM KELL\nSZÁRÍTANI";
        result.explanation=L"A páratartalom megfelelő.\nA vezérlő tovább figyeli a pincét.";
    }
    if(o.sensorWarning&&s.mode()!=Mode::Auto&&s.mode()!=Mode::Off) {
        result.explanation=L"Kézi működés! Az érzékelő hibás,\nautomatikus védelemre ne hagyatkozz.";result.tone=StatusTone::Attention;
    }
    if(o.dehumidifierIneffective&&!o.sensorWarning&&result.dehumidifierConfirmedRunning) {
        result.explanation=L"A készülék jár, de a pince nem szárad.\nEllenőrizd a készüléket és a helyiséget.";result.tone=StatusTone::Attention;
    }
    return result;
}
}
