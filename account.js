/* Shared API and account UI. Session tokens stay in HttpOnly cookies. */
(() => {
  'use strict';
  const page = location.pathname.split('/').pop();
  const protectedPages = ['dashboard.html','profile.html','history.html','Syspect.html','diagnosis.html','bsod.html','inputlag.html'];
  const labels = {
    'Scan history':'Tarama geçmişi','Sign out':'Çıkış yap','Save profile':'Profili kaydet','Display name':'Görünen ad',
    'About you':'Hakkında','Profile saved.':'Profil kaydedildi.','Saved to scan history.':'Tarama geçmişine kaydedildi.',
    'Saving report…':'Rapor kaydediliyor…','Retry save':'Kaydetmeyi yeniden dene','No scans yet.':'Henüz tarama yok.',
    'No devices yet. Run a scan to add this computer.':'Henüz cihaz yok. Bu bilgisayarı eklemek için tarama başlat.',
    'View':'İncele','Export JSON':'JSON indir','Delete':'Sil','Previous':'Önceki','Next':'Sonraki',
    'Native scan':'Motor taraması','Questionnaire':'Anket değerlendirmesi','completed':'Tamamlandı',
    'partial':'Kısmi sonuç','failed':'Başarısız','running':'Sürüyor','Devices':'Cihazlar','Rename':'Adını değiştir',
    'Change password':'Parolayı değiştir','Current password':'Mevcut parola','New password':'Yeni parola',
    'Passwords do not match.':'Parolalar eşleşmiyor.','Create a local account on this computer.':'Bu bilgisayarda yerel hesap oluştur.',
    'Saved on this computer.':'Bu bilgisayarda saklanır.','Delete this scan permanently?':'Bu tarama kalıcı olarak silinsin mi?',
    'Loading…':'Yükleniyor…','Profile':'Profil','Account':'Hesap','Results':'Sonuçlar','Close':'Kapat',
    'Scan saved.':'Tarama kaydedildi.','Open scan history':'Tarama geçmişini aç','Report could not be saved.':'Rapor kaydedilemedi.',
    'A questionnaire is based on your answers, not hardware measurements.':'Anket değerlendirmesi yanıtlarına dayanır; donanım ölçümü değildir.',
    'No result is available.':'Sonuç bulunmuyor.','Email':'E-posta','Username':'Kullanıcı adı','Created':'Oluşturulma',
    'Stored locally':'Yerel kayıt','Start a scan':'Tarama başlat','Save':'Kaydet','Cancel':'İptal',
    'Quit Syspect':'Syspect’i kapat','Syspect is closed. You can close this window.':'Syspect kapandı. Bu pencereyi kapatabilirsin.',
    'SAVED REPORTS':'KAYITLI RAPORLAR','Scan history.':'Tarama geçmişi.',
    'Your diagnostic reports, saved on this computer.':'Bu bilgisayarda saklanan teşhis raporların.',
    'Manage your account, devices and saved reports.':'Hesabını, cihazlarını ve kayıtlı raporlarını yönet.'
  };
  const errorText = {
    unauthorized:['Please sign in.','Lütfen giriş yap.'], invalid_credentials:['Email or password is incorrect.','E-posta veya parola yanlış.'],
    account_exists:['Email or username is already registered.','E-posta veya kullanıcı adı zaten kayıtlı.'],
    invalid_password:['Use 10–128 characters for your password.','Parolan 10–128 karakter olmalı.'],
    rate_limited:['Too many attempts. Try again in 15 minutes.','Çok fazla deneme. 15 dakika sonra tekrar dene.'],
    engine_timeout:['Scan timed out. You can try again.','Tarama zaman aşımına uğradı. Tekrar deneyebilirsin.'],
    interrupted:['The application closed before this scan finished.','Bu tarama tamamlanmadan uygulama kapandı.'],
    engine_not_found:['Diagnostic engine is missing.','Teşhis motoru bulunamadı.'],
    scan_in_progress:['Another scan is running.','Başka bir tarama sürüyor.']
  };
  Object.assign(errorText,{
    validation_error:['Check the information you entered.','Girdiğiniz bilgileri kontrol edin.'],
    invalid_email:['Enter a valid email address.','Geçerli bir e-posta adresi girin.'],
    invalid_username:['Use letters, numbers, dots, hyphens or underscores in your username.','Kullanıcı adında harf, rakam, nokta, kısa çizgi veya alt çizgi kullanın.'],
    engine_start_failed:['Diagnostic engine could not start.','Teşhis motoru başlatılamadı.'],
    engine_failed:['Diagnostic engine exited with an error.','Teşhis motoru hata ile kapandı.'],
    engine_output_too_large:['The scan report exceeded the size limit.','Tarama raporu boyut sınırını aştı.'],
    invalid_engine_json:['The engine response could not be read.','Motorun yanıtı okunamadı.'],
    invalid_engine_schema:['The engine report format is not supported.','Motor raporu biçimi desteklenmiyor.'],
    not_found:['The requested record was not found.','İstenen kayıt bulunamadı.'],
    conflict:['The record has changed. Refresh the page and try again.','Kayıt değişti. Sayfayı yenileyip tekrar deneyin.'],
    forbidden_origin:['This page is not allowed to access the service.','Bu sayfanın servise erişimine izin verilmedi.'],
    request_failed:['The request could not be completed. Please try again.','İstek tamamlanamadı. Lütfen tekrar deneyin.']
  });
  Object.values(errorText).forEach(([en,tr])=>{labels[en]=tr;});
  const tr = () => window.SyspectI18n?.getLanguage() === 'tr';
  const t = value => window.SyspectI18n ? window.SyspectI18n.t(value) : value;
  const errorMessage = error => (errorText[error.code] || errorText.request_failed)[tr()?1:0];
  const node = (tag, value, className) => {
    const element = document.createElement(tag);
    if (value !== undefined) element.textContent = value;
    if (className) element.className = className;
    return element;
  };
  const button = (label, handler) => {
    const element = node('button',t(label),'account-button');
    element.type = 'button'; element.onclick = handler;
    return element;
  };
  const dataNode = (tag,value,className) => {const element=node(tag,value,className);element.setAttribute('translate','no');return element;};
  async function api(path, options = {}) {
    const response = await fetch('/api/v1'+path, {
      ...options, credentials:'same-origin', cache:'no-store',
      headers:{'Content-Type':'application/json','X-Syspect-Client':'desktop',...options.headers}
    });
    const result = await response.json();
    if (!response.ok) {
      const error = new Error(result.error?.message || 'Request failed.');
      error.code = result.error?.code; error.status = response.status;
      throw error;
    }
    return result;
  }
  async function logout(event) {
    event?.preventDefault();
    try { await api('/auth/logout',{method:'POST',body:'{}'}); location.assign('login.html'); }
    catch(error) { alert(errorMessage(error)); }
  }
  function message(host, value, failed=false) {
    host.textContent = value; host.classList.toggle('account-error',failed);
  }
  function field(form,label,name,value='',type='text') {
    const wrap = node('label',t(label),'account-field');
    const input = node(type==='textarea'?'textarea':'input');
    input.name = name; input.value = value;
    if (type !== 'textarea') input.type = type;
    input.required = name !== 'bio';
    if(type==='password') {input.minLength=10;input.maxLength=128;input.autocomplete=name==='current_password'?'current-password':'new-password';}
    wrap.append(input); form.append(wrap); return input;
  }
  function info(host,label,value) {
    const item=node('div',undefined,'account-info'); item.append(node('small',t(label)),dataNode('strong',value));host.append(item);
  }
  async function profile(user) {
    const host=document.getElementById('account-content'); if(!host)return;
    host.replaceChildren();
    const hero=node('section',undefined,'account-card account-hero');
    hero.append(dataNode('div',user.display_name.slice(0,2).toUpperCase(),'account-avatar'),dataNode('h2',user.display_name),node('span',t('Saved on this computer.'),'account-muted'));
    const details=node('section',undefined,'account-card');
    const overview=node('div',undefined,'account-overview');
    info(overview,'Username',user.username);info(overview,'Email',user.email);info(overview,'Created',new Date(user.created_at).toLocaleDateString(tr()?'tr-TR':'en-US'));overview.lastElementChild.querySelector('strong').dataset.createdAt=user.created_at;
    details.append(overview);
    const form=node('form'); const displayName=field(form,'Display name','display_name',user.display_name);displayName.maxLength=80;
    field(form,'About you','bio',user.bio,'textarea').maxLength=500;
    const save=node('button',t('Save profile'),'account-button'); save.type='submit';
    const status=node('p','', 'account-status');status.setAttribute('role','status');form.append(save,status);
    form.onsubmit=async event=>{event.preventDefault();save.disabled=true;try {const data=await api('/profile',{method:'PATCH',body:JSON.stringify(Object.fromEntries(new FormData(form)))});window.SyspectAccount.user=data.user;hero.querySelector('h2').textContent=data.user.display_name;message(status,t('Profile saved.'));}catch(error){message(status,errorMessage(error),true);}finally{save.disabled=false;}};
    details.append(form);
    const passwordCard=node('section',undefined,'account-card');passwordCard.append(node('h2',t('Change password')));
    const passwordForm=node('form');field(passwordForm,'Current password','current_password','','password');field(passwordForm,'New password','new_password','','password');
    const change=node('button',t('Change password'),'account-button');change.type='submit';const passwordStatus=node('p','','account-status');passwordStatus.setAttribute('role','status');passwordForm.append(change,passwordStatus);
    passwordForm.onsubmit=async event=>{event.preventDefault();change.disabled=true;try{await api('/auth/password',{method:'POST',body:JSON.stringify(Object.fromEntries(new FormData(passwordForm)))});location.assign('login.html');}catch(error){message(passwordStatus,errorMessage(error),true);}finally{change.disabled=false;}};
    passwordCard.append(passwordForm);
    host.append(hero,details,passwordCard);
    await devices(host);
  }
  async function devices(host) {
    const card=node('section',undefined,'account-card');card.append(node('h2',t('Devices')));host.append(card);
    const data=await api('/devices');
    if(!data.devices.length)card.append(node('p',t('No devices yet. Run a scan to add this computer.')));
    for(const device of data.devices){
      const row=node('div',undefined,'account-device');const name=dataNode('strong',device.name);
      row.append(name,node('p',[device.system.cpu,device.system.ram_gb?device.system.ram_gb+' GB':null,device.system.windows_build?'Windows '+device.system.windows_build:null].filter(Boolean).join(' · ')));
      row.append(button('Rename',()=>{
        const form=node('form');const input=field(form,'Devices','name',device.name);input.maxLength=80;
        const save=node('button',t('Save'),'account-button');save.type='submit';const status=node('p');form.append(save,status);
        form.onsubmit=async event=>{event.preventDefault();save.disabled=true;try{await api('/devices/'+device.id,{method:'PATCH',body:JSON.stringify({name:input.value})});device.name=input.value;name.textContent=input.value;form.remove();}catch(error){status.textContent=errorMessage(error);}finally{save.disabled=false;}};
        row.append(form);
      }));card.append(row);
    }
  }
  let historyOffset=0, historyTimer;
  async function history(){
    const host=document.getElementById('scan-history');if(!host)return;
    clearTimeout(historyTimer);
    const data=await api('/scans?limit=10&offset='+historyOffset);
    host.replaceChildren();
    if(!data.scans.length)host.append(node('p',t('No scans yet.')));
    for(const scan of data.scans){
      const row=node('article',undefined,'account-scan');
      const heading=node('div');const scanTitle=node('h3');scanTitle.append(node('span',t(scan.source==='native'?'Native scan':'Questionnaire')));
      if(scan.source!=='native')scanTitle.append(' · ',node('span',t(({fpsDrops:'FPS Drops',overheating:'Overheating',bsod:'Blue Screen (BSOD)',inputlag:'Slow Boot & System Lag'})[scan.scan_type]||'Questionnaire')));
      heading.append(scanTitle,dataNode('p',new Date(scan.created_at).toLocaleString(tr()?'tr-TR':'en-US')+' · '+scan.device_name));
      row.append(heading,node('span',t(scan.status),'account-badge '+scan.status));
      if(scan.error_code)row.append(node('p',errorMessage({code:scan.error_code,message:scan.error_code}),'account-error'));
      const actions=node('div',undefined,'account-actions');
      actions.append(button('View',()=>showScan(scan.id).catch(error=>alert(errorMessage(error)))));
      const link=node('a',t('Export JSON'),'account-button');link.href='/api/v1/scans/'+scan.id+'/export';link.download='syspect-'+scan.id+'.json';actions.append(link);
      if(scan.status!=='running')actions.append(button('Delete',async()=>{if(!confirm(t('Delete this scan permanently?')))return;try{await api('/scans/'+scan.id,{method:'DELETE',body:'{}'});await history();}catch(error){alert(errorMessage(error));}}));
      row.append(actions);host.append(row);
    }
    const nav=node('div',undefined,'account-actions');const prev=button('Previous',()=>{historyOffset=Math.max(0,historyOffset-10);history().catch(showHistoryError);});prev.disabled=historyOffset===0;
    const next=button('Next',()=>{historyOffset+=10;history().catch(showHistoryError);});next.disabled=historyOffset+data.scans.length>=data.total;
    nav.append(prev,node('span',`${data.total} ${t('Results')}`),next);host.append(nav);
    if(data.scans.some(scan=>scan.status==='running'))historyTimer=setTimeout(()=>history().catch(showHistoryError),2500);
  }
  function showHistoryError(error){const host=document.getElementById('scan-history');if(host)message(host,errorMessage(error),true);}
  async function showScan(id){
    const {scan}=await api('/scans/'+id);const host=document.getElementById('scan-detail');host.replaceChildren();host.hidden=false;
    host.append(button('Close',()=>{host.hidden=true;}),node('h2',t('Results')),node('p',new Date(scan.created_at).toLocaleString(tr()?'tr-TR':'en-US')));
    if(scan.source==='questionnaire')host.append(node('p',t('A questionnaire is based on your answers, not hardware measurements.')));
    const pre=node('pre',scan.result ? (scan.source==='native'?scan.result.raw_report:JSON.stringify(scan.result,null,2)) : errorMessage({code:scan.error_code,message:scan.error_code||t('No result is available.')}));
    pre.setAttribute('translate','no');host.append(pre);host.scrollIntoView({behavior:'smooth',block:'start'});
  }
  async function saveQuestionnaire(scanType,state){
    const host=document.querySelector('.results-card');if(!host)return;
    const status=node('p',t('Saving report…'),'account-status');status.setAttribute('role','status');host.append(status);
    const payload={source:'questionnaire',scan_type:scanType,request_id:crypto.randomUUID(),result:{...structuredClone(state.results),answers:structuredClone(state.answers)}};
    async function save(){
      message(status,t('Saving report…'));
      try{await api('/scans',{method:'POST',body:JSON.stringify(payload)});message(status,t('Saved to scan history.'));const link=node('a',t('Open scan history'));link.href='history.html';status.append(' ',link);}
      catch(error){message(status,t('Report could not be saved.'),true);status.append(button('Retry save',save));}
    }
    await save();
  }
  async function scanNative(onProgress){
    const {scan}=await api('/scans',{method:'POST',body:JSON.stringify({source:'native'})});
    let current=scan;
    while(current.status==='running'){
      await new Promise(resolve=>setTimeout(resolve,1500));
      current=(await api('/scans/'+scan.id)).scan;onProgress?.(current);
    }
    if(current.status==='failed'){const error=new Error(current.error_code);error.code=current.error_code;throw error;}
    return current.result;
  }
  async function init(){
    if(page==='login.html'||page==='register.html'){
      const form=document.querySelector('form');const register=page==='register.html';
      const inputs=form.querySelectorAll('input');const names=register?['username','email','password','confirm_password']:['email','password'];
      inputs.forEach((input,i)=>{input.name=names[i];input.id=names[i];input.previousElementSibling?.setAttribute('for',input.id);input.autocomplete=input.type==='password'?(register?'new-password':'current-password'):(input.type==='email'?'email':'username');if(input.type==='password'){input.minLength=10;input.maxLength=128;}});
      const status=node('p','','account-status');status.setAttribute('role','alert');form.append(status);
      form.onsubmit=async event=>{event.preventDefault();const data=Object.fromEntries(new FormData(form));const submit=form.querySelector('button');if(register&&data.password!==data.confirm_password){message(status,t('Passwords do not match.'),true);return;}delete data.confirm_password;submit.disabled=true;try{await api('/auth/'+(register?'register':'login'),{method:'POST',body:JSON.stringify(data)});location.assign('dashboard.html');}catch(error){message(status,errorMessage(error),true);}finally{submit.disabled=false;}};
      document.querySelector('.auth-subtitle').textContent=t(register?'Create a local account on this computer.':'Saved on this computer.');
    }
    let user;
    try{user=(await api('/auth/me')).user;}
    catch(error){if(error.status===401){if(protectedPages.includes(page))location.replace('login.html');return;}if(protectedPages.includes(page))throw error;return;}
    window.SyspectAccount.user=user;
    document.querySelectorAll('.welcome-msg span').forEach(item=>{item.setAttribute('translate','no');item.textContent=user.display_name;});
    document.querySelectorAll('.avatar').forEach(item=>{item.setAttribute('translate','no');item.textContent=user.display_name.slice(0,2).toUpperCase();});
    document.querySelectorAll('.btn-logout').forEach(item=>item.onclick=logout);
    const top=document.querySelector('.top-right')||document.querySelector('.topbar');
    if(top){const link=node('a',t('Scan history'),'account-nav');link.href='history.html';top.append(link);if(!document.querySelector('.btn-logout'))top.append(button('Sign out',logout));top.append(button('Quit Syspect',async()=>{try{await api('/app/exit',{method:'POST',body:'{}'});document.body.replaceChildren(node('p',t('Syspect is closed. You can close this window.')));}catch(error){alert(errorMessage(error));}}));}
    if(page==='profile.html')await profile(user);
    if(page==='history.html')await history();
    if(page==='dashboard.html'){
      const data=await api('/scans?limit=1');const card=node('section',undefined,'account-card');card.append(node('h2',t('Scan history')),node('p',data.total+' '+t('Results')));const link=node('a',t('Open scan history'),'account-button');link.href='history.html';card.append(link);document.querySelector('main')?.prepend(card);
      const deviceHost=document.getElementById('dashboard-device');
      const {devices:items}=await api('/devices');
      if(deviceHost){deviceHost.replaceChildren();deviceHost.append(node('h2',t('Devices')));if(!items.length){deviceHost.append(node('p',t('No devices yet. Run a scan to add this computer.')));}else{for(const device of items){deviceHost.append(dataNode('h3',device.name),node('p',[device.system.cpu,device.system.ram_gb?device.system.ram_gb+' GB':null,device.system.windows_build?'Windows '+device.system.windows_build:null].filter(Boolean).join(' · ')||t('No result is available.')));}}}
    }
  }
  window.SyspectAccount={api,scanNative,saveQuestionnaire,errorMessage,user:null};
  window.SyspectAccountTranslations=labels;
  window.addEventListener('syspect-language-change',()=>{
    if(page==='history.html'&&window.SyspectAccount.user)history().catch(showHistoryError);
    document.querySelectorAll('[data-created-at]').forEach(el=>{el.textContent=new Date(el.dataset.createdAt).toLocaleDateString(tr()?'tr-TR':'en-US');});
    // Preserve form values when toggling language on profile and authentication pages.
    document.querySelectorAll('.account-button,.account-field,.account-info small,.account-muted,.account-nav').forEach(element=>{
      const reverse=Object.fromEntries(Object.entries(labels).map(([en,value])=>[value,en]));
      for(const child of element.childNodes)if(child.nodeType===Node.TEXT_NODE){const key=reverse[child.textContent]||child.textContent;child.textContent=t(key);}
    });
  });
  document.addEventListener('DOMContentLoaded',()=>init().catch(error=>{const host=document.querySelector('main')||document.body;host.prepend(node('p',errorMessage(error),'account-error'));}));
})();
