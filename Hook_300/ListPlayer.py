# -*- coding:utf-8 -*-
#python program by kami@maikaze.win
#first finish on 20160511

import urllib.request
import urllib.parse
import re
import sys
import threading

allykda = enemykda = id_index = 0
allynum = enemynum = 0
#from http://www.cnblogs.com/mfrbuaa/p/4600848.html
#author: Song Bo, Eagle, ZJU
#email: sbo@zju.edu.cn

def myAlign(string, length=0):
	if length == 0:
		return string
	slen = len(string)
	re = string
	placeholder = u'　'
	while slen < length:
		re += placeholder
		slen += 1
	return re
#end
  
def getHtml(url):
  page = urllib.request.urlopen(url)
  html = page.read()
  html = html.decode('UTF-8')
  return html

def getRecords(id):
  id_url = urllib.parse.quote(id)
  index = 0
  sumResult = []
  global allynum
  global enemynum
  while True:
    url = "http://300report.jumpw.com/api/getlist?name=" + id_url + "&index=" + str(index)
    index += 10
    findPage = getHtml(url)  
    #judge if the id is available
    reqid = re.compile(r'MatchID":(\d{8}),"MatchType":1.*?MatchDate":"(.{10})')
    reqResult = re.findall(reqid, findPage)  
    #if there still result exists
    if reqResult:
      sumResult = sumResult + reqResult
    else:
      break
  if sumResult:
    #start the analyze
    #print ("Get the record of player->" + id)
    #sum up
    kill = death = assist = singlekda = tmpdeath = 0
    for a, b in sumResult[::-1]:
      kda = getKDA(a, id)
      
      kill = kill + int(kda[0][1])
      death = death + int(kda[0][2])
      assist = assist + int(kda[0][3])
      
    #deal if death is zero
    if (death == 0):
      death = 1
    avgkda = (kill + assist) * 3 / death
    if avgkda < 4:
      star = '☆'
    elif avgkda < 6:    
      star = '☆☆'
    elif avgkda < 8:    
      star = '☆☆☆'
    elif avgkda < 10:    
      star = '☆☆☆☆'
    else:    
      star = '☆☆☆☆☆'
    print ('kda of ' + id + ' recently ' + str(len(sumResult)) + ' matches is :' + str("%.2f" % avgkda) + '   ' + star)
    
  else:
    #report the error
    print ("cannot find the player->" + id)
    avgkda = 0
    
  global allykda
  global enemykda
  global id_index
  
  id_index += 1
  if id_index > 7:
    enemykda += avgkda
    enemynum += 1
  else:
    allykda += avgkda
    allynum += 1
  return avgkda
  
def getKDA(match, id):
  url = 'http://300report.jumpw.com/api/getmatch?id=' + match
  matchPage = getHtml(url)
  reqRule = id + r'.*?mResult":(\d),".*?killCount":(\d{1,}).*?(\d{1,}).*?(\d{1,}).*?"KDA":(.*?),.*?"Hero":.*?me":"(.*?)",'
  reqPlayer = re.compile(reqRule, re.I)
  reqkda = re.findall(reqPlayer,matchPage)
  return reqkda
  
#start
if __name__=='__main__':  
  threads = []
  for i in range(1, 8):
    threads.append(threading.Thread(target=getRecords, args=(sys.argv[i],)));
    
  print ('====Maikaze Python Program for Looking up your recent score in 300 Heros====')
  print ('                       ====Powered in 20160508====')
  print ('Ally:')
  for t in threads:
    t.start()
  for t in threads:
    t.join()
  
  print ('Enemy:')
  threads.clear()  
  for i in range(8, len(sys.argv)):
    threads.append(threading.Thread(target=getRecords, args=(sys.argv[i],)));
  for t in threads:
    t.start()
  for t in threads:
    t.join()
  
  rate = (allykda / allynum) / (allykda / allynum + enemykda / enemynum)
  print ("win rate:" + str(rate))