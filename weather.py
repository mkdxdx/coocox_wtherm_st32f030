import pycurl,json,datetime,math,time
from io import BytesIO

# this string should be configured according to your ESP-01 config
thermaddr = "http://station.ip:port"
curlstring = 'http://api.openweathermap.org/data/2.5/forecast?id=CITYID&units=metric&appid=APPID'

running = True
maxindex = 4
maxtries = 5
wind_arr = [
    [22, 67, ord("N"), ord("E")],
    [68, 112, ord(" "), ord("E")],
    [113, 157, ord("S"), ord("E")],
    [158, 202, ord(" "), ord("S")],
    [203, 247, ord("S"), ord("W")],
    [248, 292, ord(" "), ord("W")],
    [293, 337, ord("N"), ord("W")],
    [338, 383, ord(" "), ord("N")],
]

while(running == True):
    print("Sending:")

    today = datetime.datetime.now().day
    hnow = datetime.datetime.now().hour

    wind_spd = 0;
    wind_dir = [0,0]

    buffer = BytesIO()
    curl = pycurl.Curl()
    curl.setopt(curl.URL, curlstring)
    curl.setopt(curl.WRITEDATA, buffer)
    curl.perform()
    curl.close()
    fcaststr = buffer.getvalue()
    forecast = json.loads(fcaststr.decode('utf-8'))
    city_name = forecast["city"]["name"]
    weather_array = []
    wlist = forecast["list"]

    temp_warray = {}

    ind = 0
    for fcast in wlist:
        tdat = datetime.datetime.fromtimestamp(fcast["dt"])
        day = tdat.day
        hr = tdat.hour
        if day > today:
            main = fcast["main"]
            warr = [main["temp"], fcast["clouds"]["all"], ind-1, fcast["main"]["humidity"]]
            if day in temp_warray:
                temp_warray[day].append(warr)
            else:
                ind = ind + 1
                temp_warray[day] = []
                temp_warray[day].append(warr)
            #print(str(day)+" day added")
        elif (day == today and hr>=hnow and hr<=hnow+3):
            ws = math.floor(fcast["wind"]["speed"])
            wd = math.floor(fcast["wind"]["deg"])
            if wd < 21:
                wd = wd + 338

            wind_spd = ws

            for direction in wind_arr:
                if direction[0] <= wd <= direction[1]:
                    wind_dir[0] = direction[2];
                    wind_dir[1] = direction[3];
                    break



    for date,day_w in temp_warray.items():
        tmin = day_w[0][0]
        tmax = day_w[0][0]
        idx = day_w[1][2]
        clo = day_w[4][1]
        hum = day_w[0][3]
        if idx<maxindex:
            for hr_w in day_w:
                tmin = min(tmin,hr_w[0])
                tmax = max(tmax,hr_w[0])
                hum = max(hum,hr_w[3])

            tmin = math.floor(tmin)
            tmax = math.ceil(tmax)

            tl_sign = ord('+')
            th_sign = ord('+')

            if (tmax < 0):
                th_sign = ord('-')
            if (tmin < 0):
                tl_sign = ord('-')

            #print(str(idx)+" DAY "+""+str(date)+" / "+str(tmin)+" ~ "+str(tmax))
            weather_array.append([idx,date,th_sign,abs(tmax),tl_sign,abs(tmin),hum,clo])


        #weather_array.append


    postcmd = []

    for weather in weather_array:
        print(str(weather[0])+": D"+str(weather[1])+" "+chr(weather[2])+str(weather[3])+" ~ "+chr(weather[4])+str(weather[5])+" C RH:"+str(weather[6])+"% V:"+str(weather[7])+"%")
        fcaststr = "FCS;"+chr(weather[0])+chr(weather[1])+chr(weather[2])+chr(weather[3])+chr(weather[4])+chr(weather[5])+chr(weather[6])+chr(weather[7])
        postcmd.append(fcaststr)

    postcmd.append("SWS;" + chr(wind_spd) + chr(wind_dir[0]) + chr(wind_dir[1]))

    for attempts in range(1,maxtries):
        for cmd in postcmd:
            try:
                co = pycurl.Curl()
                co.setopt(co.URL,thermaddr)
                co.setopt(co.TIMEOUT,1)
                co.setopt(co.USERAGENT,"")
                co.setopt(co.POST,1)
                co.setopt(co.POSTFIELDS,cmd)
                co.setopt(co.HTTPHEADER,["Content-Type: application/octet-stream"])
                co.perform()
                co.close()
                print(cmd)
            except pycurl.error:
                pass
            finally:
               time.sleep(2)
    time.sleep(10)

