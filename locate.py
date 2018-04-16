#author zhangjian
#date 2018.4.16

def getAbsCoor(p):
    ans=()
    if p=='C'    : ans=(0,0)
    elif p=='P1' : ans=(-52.5,-32)
    elif p=='P2' : ans=(-52.5, 32)
    elif p=='P3' : ans=(52.5,32)
    elif p=='P4' : ans=(52.5,-32)
    elif p=='P5': ans=(0,-32)
    elif p=='P6' : ans=(0,32)
    elif p=='P7' : ans=(-30,-7)
    elif p=='P8' : ans=(-30, 7)
    elif p=='P9' : ans=(30,7)
    elif p=='P10': ans=(30, -7)
    elif p=='G1' : ans=(-52.5,0)
    elif p=='G2' : ans=(52.5,0)
    return ans

def calDist(a,b):
    x_s=(a[0]-b[0])**2
    y_s=(a[1]-b[1])**2
    return (x_s+y_s)**0.5
    
info='(P8 22 0) (P7 15 30)'

inl=info.split()

f_r=float(inl[1])
g_r=float(inl[4])
f_a=float(inl[2].strip(')'))
g_a=float(inl[5].strip(')'))
f_c=getAbsCoor(inl[0].strip('('))
g_c=getAbsCoor(inl[3].strip('('))

d=calDist(f_c,g_c)
a=(f_r**2-g_r**2+d**2)/(2*d)
h=(f_r**2-a**2)**0.5
cos=(g_c[0]-f_c[0])/d
sin=(g_c[1]-f_c[1])/d
sign=-1 if g_a-f_a>0 else 1

p_x_=f_c[0]+a*cos
p_y_=f_c[1]+a*sin

p_x=p_x_+h*sign*sin
p_y=p_y_-h*sign*cos
ans=(p_x,p_y)

print('2个标记:',info)
print('全局坐标:',ans)
