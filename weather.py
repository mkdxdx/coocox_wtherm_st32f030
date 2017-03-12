# weather station update script
# by mkdxdx, MIT license

import pycurl,json,datetime,math,time
from io import BytesIO

# this string should be configured according to your ESP-01 config
thermaddr = "http://station.ip:port"
curlstring = 'http://api.openweathermap.org/data/2.5/forecast?id=YOURCITYID&units=metric&appid=YOURAPPID'

running = True
maxindex = 4
maxtries = 3
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
    for fcast in wlist:
        tdat = datetime.datetime.fromtimestamp(fcast["dt"])
        day = tdat.day
        hour = tdat.hour

        if day>today and hour > 11 and hour <15:
            main = fcast["main"]
            t_min = math.floor(main["temp_min"])
            t_max = math.ceil(main["temp_max"])

            tl_sign = ord('+')
            th_sign = ord('+')

            if (t_max<0):
                th_sign = ord('-')
            if (t_min<0):
                tl_sign = ord('-')


            hum = math.floor(main["humidity"])
            clouds = fcast["clouds"]["all"]
            weather_array.append([day, th_sign, abs(t_max), tl_sign, abs(t_min), hum, clouds])
        elif (day == today and hour>=hnow and hour<=hnow+3):
            ws = math.floor(fcast["wind"]["speed"])
            wd = math.floor(fcast["wind"]["deg"])
            if wd<21:
                wd = wd + 338

            wind_spd = ws

            for direction in wind_arr:
                if direction[0]<=wd<=direction[1]:
                    wind_dir[0] = direction[2];
                    wind_dir[1] = direction[3];
                    break
            print("WIND@"+str(hour)+":"+str(wind_spd)+"ms / "+chr(wind_dir[0])+chr(wind_dir[1]))


    index = 0

    postcmd = []

    for weather in weather_array:
        clo = weather[6]
        print(str(weather[0])+"\tT:"+chr(weather[1])+str(weather[2])+" ~ "+chr(weather[3])+str(weather[4])+"C\tRH:"+str(weather[5])+"%\t"+str(clo))
        fcaststr = "FCS;"+chr(index)+chr(weather[0])+chr(weather[1])+chr(weather[2])+chr(weather[3])+chr(weather[4])+chr(weather[5])+chr(weather[6])
        index = index + 1
        postcmd.append(fcaststr)

        if index >= maxindex:
            #append wind command
            postcmd.append("SWS;"+chr(wind_spd)+chr(wind_dir[0])+chr(wind_dir[1]))
            break

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

