# 实验考核题-2026 参考代码（基于 fiftyExp.cpp 风格）

/*
说明的说明：
    下面的说明是本人在研究考核题代码时遇到的问题的一些相关解答，并包含一些对前人代码的修正，可以
    帮助你了解 uva 框架的部分常见函数的使用方法。
    如果后续年份的题目发生改变，可以把题目、这篇代码以及课程提供的uva框架源代码交给AI，从而获得
    对新题目的不错解答~
*/

/*
说明：
- 拿球策略写在 PlayerTeams.cpp 的 deMeer5() 约134行 `else if(WM->isBallKickable())` 分支内；
- 无球跑位/防守策略写在约180行（把原来的 `soc = turnBodyToObject(OBJECT_BALL)` 改掉）；
- 每次修改后要重新构建才能看到效果。

【已对照 uva_base_v13.2 源码核实】
- dribble(ang,...) 的 ang 是**全局角度**（源码实现：angDiff = normalizeAngle(ang - angBody)），
  fiftyExp 里传 getDirection() 全局方向的写法是对的；dribble(0.0) 即朝对方球门方向。
- kickBallCloseToBody(ang) 的 ang 才是相对身体角度（源码：angGlobal = angBody + ang）。
- moveToPos(pos, angWhenToTurn) 第二参数是"转身阈值角度"，不是速度。
- **界外球模式应该用 isKickInUs()/isKickInThem()**！isOffsideUs/isOffsideThem 是"越位"，
  fiftyExp 用它们表示界外球是错的（2D里界外球就是kick-in）。本文件已全部改为 isKickInUs/Them。
- getMarkingPosition 第一个参数是 VecPosition（盯防对象的位置），不是 ObjectT！
  fiftyExp 直接传 OBJECT_OPPONENT_x 会被隐式转换成错误坐标，本文件已改为传 getGlobalPosition()。
- VecPosition 有 POLAR 极坐标构造、setX/setY；isCornerKickThem、PITCH_WIDTH 均存在。
- ShootToGoalex 函数在源码中不存在，射门一律用 kickTo。

---
*/


// 一、playOn 模式下 · 拿球后进攻行为

//1. 朝自身正前方快速带球
soc = dribble(0.0,DRIBBLE_FAST);
//【源码核实】dribble第一个参数是全局角度，0度即朝对方球门方向（开球时身体正对球门，等价于正前方）

//2. 朝对方球门方向慢速带球
AngDeg posDribble = (VecPosition(52.5,0) - posAgent).getDirection();
soc = dribble(posDribble,DRIBBLE_SLOW);

//3. 连续短踢，控制球绕自身身体逆时针圆周运动
soc = kickBallCloseToBody(-120);   //fiftyExp中逆时针参数为-120

//4. 7米内有对方逼抢，则踢到逼抢对手的另一侧安全带球
Circle cir(posAgent,7);
int num = WM->getNrInSetInCircle(OBJECT_SET_OPPONENTS,cir);
AngDeg ang = 0;
if(num > 0){
    ObjectT o = WM->getClosestInSetTo(OBJECT_SET_OPPONENTS,posAgent); //最近的对手
    VecPosition p = WM->getGlobalPosition(o);
    if((p-posAgent).getDirection() >= 0)   //对手在右侧（角度为正）
        ang -= 45;                        //【修正】踢向左侧；fiftyExp原文写反了(+=45是同侧)
    else                                  //对手在左侧
        ang += 45;                        //踢向右侧
    soc = dribble(ang,DRIBBLE_WITHBALL);
}
else{
    soc = dribble((VecPosition(52.5,0)-posAgent).getDirection(),DRIBBLE_WITHBALL);
}

//5. 7米内有逼抢则传给最近队友；否则向对方球门快速带球
Circle cir(posAgent,7);
int num = WM->getNrInSetInCircle(OBJECT_SET_OPPONENTS,cir);
if(num > 0){
    soc = leadingPass(WM->getClosestInSetTo(OBJECT_SET_TEAMMATES,posAgent),1.0);
    //leadingPass第二参数为传球速度/力度系数（源程序中查）
}
else{
    soc = dribble((VecPosition(52.5,0)-posAgent).getDirection(),DRIBBLE_FAST);
}

//6. 7米内有逼抢则安全带球；否则向对方球门快速带球
Circle cir(posAgent,7);
int num = WM->getNrInSetInCircle(OBJECT_SET_OPPONENTS,cir);
AngDeg ang = (VecPosition(52.5,0)-posAgent).getDirection();
if(num > 0)
    soc = dribble(0.0,DRIBBLE_WITHBALL);   //fiftyExp写法：原地安全带球
else
    soc = dribble(ang,DRIBBLE_FAST);

//7. 前方无对方球员则最大力量射门；偶数周期射右侧，奇数周期射左侧
//【源码核实】isOpponentAtAngle(ang,dDist)第一参数是方向角、第二是距离，窗口±60°写死，不符合题意。
//改用自加的isOpponentAtAngleEx(angA,angB,dDist)：判断角度[angA,angB]、距离dDist内有无对手
//注意：angA/angB是全局角度，(-30,30)表示"朝对方球门方向±30°"的扇形区域
if(WM->isOpponentAtAngleEx(-30,30,30) == false){   //前方±30°、30米内无对手
    VecPosition posGoal( PITCH_LENGTH/2.0,
        (-1 + 2*(WM->getCurrentCycle()%2)) * 0.4 * SS->getGoalWidth() );
    //【修正】必须是 2*(cycle%2)，写成 2*cycle%2 会因优先级恒等于0
    //getGoalWidth()表示球门宽度，乘0.4防止踢出门外
    soc = kickTo(posGoal, SS->getBallSpeedMax());
}

//8. 本方半场踢向中心点(0,0)；对方半场快速朝球门带球
if(WM->getBallPos().getX() < 0){
    soc = kickTo(VecPosition(0,0),1.0);
}
else{
    soc = dribble((VecPosition(52.5,0)-posAgent).getDirection(),DRIBBLE_FAST); //【修正】VecPosition(52.5,0)
}

//9. 传给距离最近且周围5米内无对方球员的队友
ObjectT ClosestAgent = WM->getClosestInSetTo(OBJECT_SET_TEAMMATES,posAgent);
Circle cir(WM->getGlobalPosition(ClosestAgent),5);   //【修正】Circle需要VecPosition
int num = WM->getNrInSetInCircle(OBJECT_SET_OPPONENTS,cir);
if(num == 0){
    soc = leadingPass(ClosestAgent,1.0);
}
else soc = dribble(0.0,DRIBBLE_WITHBALL);

//10. 传给最近、在自己前方、身前5米无对方防守的队友
ObjectT ClosestAgent = WM->getClosestInSetTo(OBJECT_SET_TEAMMATES,posAgent);
AngDeg ang = (WM->getGlobalPosition(ClosestAgent)-posAgent).getDirection();
Circle cir(WM->getGlobalPosition(ClosestAgent),5);
int num = WM->getNrInSetInCircle(OBJECT_SET_OPPONENTS,cir);
if(ang > -90 && ang < 90 && num == 0){   //【修正】fiftyExp原文 ang>90 是错的
    soc = leadingPass(ClosestAgent,1.0);
}
else soc = dribble((VecPosition(52.5,0)-posAgent).getDirection(),DRIBBLE_WITHBALL);

//11. 本方半场传给次近队友；对方半场10号带球，非10号传给10号
if(WM->getBallPos().getX() < 0){   //【修正】getBallPos().getX()，不是getBallPosition()<0
    soc = leadingPass(WM->getSecondClosestInSetTo(OBJECT_SET_TEAMMATES,WM->getAgentObjectType()),1.0);
}
else{
    if(WM->getPlayerNumber() == 10)
        soc = dribble((VecPosition(52.5,0)-posAgent).getDirection(),DRIBBLE_FAST);
    else
        soc = leadingPass(OBJECT_TEAMMATE_10,1.0);
}

//12. 本方半场朝自身前进方向慢速带球；对方半场朝球门快速带球
if(WM->getBallPos().getX() < 0)
    soc = dribble(0.0,DRIBBLE_SLOW);
else
    soc = dribble((VecPosition(52.5,0)-posAgent).getDirection(),DRIBBLE_FAST);

//13. 2号踢向左侧边线；5号踢向右侧边线并转脖子看球；其余朝前带球
if(WM->getPlayerNumber() == 2)
    soc = kickTo(VecPosition(WM->getBallPos().getX(),-34),1.0);
else if(WM->getPlayerNumber() == 5){
    soc = kickTo(VecPosition(WM->getBallPos().getX(),34),1.0);
    //提交命令后追加：ACT->putCommandInQueue(turnNeckToObject(OBJECT_BALL,soc)); 转脖子看球
}
else soc = dribble(0.0,DRIBBLE_WITHBALL);

//14. 禁区内最大力量射向空隙大的一侧（判断夹角）；否则4号传7号，其余传最近队友
if(WM->isInTheirPenaltyArea(WM->getBallPos())){
    VecPosition posGoalie = WM->getGlobalPosition(WM->getOppGoalieType());
    AngDeg ang_goalie = (posGoalie-posAgent).getDirection();
    AngDeg angup   = (VecPosition(52.5, 6.0)-posAgent).getDirection();
    AngDeg angdown = (VecPosition(52.5,-6.0)-posAgent).getDirection();
    if(std::fabs(angup-ang_goalie) > std::fabs(angdown-ang_goalie))
        soc = kickTo(VecPosition(52.5, 6.0),SS->getBallSpeedMax());
    else
        soc = kickTo(VecPosition(52.5,-6.0),SS->getBallSpeedMax());
}
else{   //【修正】fiftyExp原文这里漏了else，会覆盖上面的射门
    if(WM->getPlayerNumber() == 4)
        soc = leadingPass(OBJECT_TEAMMATE_7,1.0);
    else
        soc = leadingPass(WM->getClosestInSetTo(OBJECT_SET_TEAMMATES,posAgent),1.0);
}

//15. 沿垂直于X轴方向（横向）带球
if(WM->getBallPos().getY() <= 0)
    soc = dribble(-90,DRIBBLE_WITHBALL);
else
    soc = dribble(90,DRIBBLE_WITHBALL);

//16. 带球到球场中心点；距中心2米内传给最近队友
if(posAgent.getDistanceTo(VecPosition(0,0)) > 2.0){   //【修正】应判断到中心点的距离
    soc = dribble((VecPosition(0,0)-posAgent).getDirection(),DRIBBLE_WITHBALL);
}
else{
    soc = leadingPass(WM->getClosestInSetTo(OBJECT_SET_TEAMMATES,posAgent),1.0);
}

//17. 10号可踢球且x>30，朝距对方防守球员（守门员）更远的球门点射门（旧20题）
if(WM->getPlayerNumber() == 10 && posAgent.getX() > 30){
    VecPosition posGoalie = WM->getGlobalPosition(WM->getOppGoalieType());
    AngDeg ang_goalie = (posGoalie-posAgent).getDirection();
    AngDeg angup   = (VecPosition(52.5, 6.0)-posAgent).getDirection();
    AngDeg angdown = (VecPosition(52.5,-6.0)-posAgent).getDirection();
    if(std::fabs(angup-ang_goalie) > std::fabs(angdown-ang_goalie))  //【修正】fabs不是fab
        soc = kickTo(VecPosition(52.5, 6.0),SS->getBallSpeedMax());
    else
        soc = kickTo(VecPosition(52.5,-6.0),SS->getBallSpeedMax());
}
else{
    soc = leadingPass(OBJECT_TEAMMATE_10,1.0);
}

//18. 9号带球、10号无球跑位共同冲向球门；到球门前9号传10号射门（旧24题）
//--- 134行拿球策略 ---
if(WM->getPlayerNumber() == 9){
    if(WM->isInTheirPenaltyArea(WM->getBallPos()))
        soc = leadingPass(OBJECT_TEAMMATE_10,1.0);
    else
        soc = dribble((VecPosition(52.5,0)-posAgent).getDirection(),DRIBBLE_FAST);
}
else if(WM->getPlayerNumber() == 10){
    if(WM->isInTheirPenaltyArea(WM->getBallPos())){
        //向空隙大的一侧射门
        VecPosition posGoalie = WM->getGlobalPosition(WM->getOppGoalieType());
        AngDeg ang_goalie = (posGoalie-posAgent).getDirection();
        AngDeg angup   = (VecPosition(52.5, 6.0)-posAgent).getDirection();
        AngDeg angdown = (VecPosition(52.5,-6.0)-posAgent).getDirection();
        if(std::fabs(angup-ang_goalie) > std::fabs(angdown-ang_goalie))
            soc = kickTo(VecPosition(52.5, 6.0),SS->getBallSpeedMax());
        else
            soc = kickTo(VecPosition(52.5,-6.0),SS->getBallSpeedMax());
    }
    else
        soc = leadingPass(OBJECT_TEAMMATE_9,1.0);
}
else{
    soc = leadingPass(OBJECT_TEAMMATE_9,1.0);   //其余球员把球交给9号
}
//--- 180行跑位策略 ---
if(WM->getPlayerNumber() == 10){
    soc = moveToPos(VecPosition(52.5,0),20);   //10号无球冲向球门
}

//19. 4号传7号；7号可踢球传9号；9号拿球后最大速度冲向球门射门（旧25题）
if(WM->getPlayerNumber() == 4)
    soc = leadingPass(OBJECT_TEAMMATE_7,1.0);
else if(WM->getPlayerNumber() == 7)
    soc = leadingPass(OBJECT_TEAMMATE_9,1.0);
else if(WM->getPlayerNumber() == 9){
    if(WM->getBallPos().getX() < 40)
        soc = dribble((VecPosition(40,0)-posAgent).getDirection(),DRIBBLE_FAST);
    else
        soc = kickTo(VecPosition(52.5,0),SS->getBallSpeedMax());
        //【源码核实】fiftyExp原文的ShootToGoalex在uva_base源码中不存在，射门用kickTo
}
else
    soc = leadingPass(OBJECT_TEAMMATE_4,1.0);

//20. 本方半场最大力量踢向中场线；对方半场最大力量踢向对方球门（旧23题）
if(WM->getBallPos().getX() <= 0){
    VecPosition pos(0,WM->getBallPos().getY());   //【修正】fiftyExp原文grtBallPos是拼写错误
    soc = kickTo(pos,SS->getBallSpeedMax());
}
else{
    soc = kickTo(VecPosition(52.5,0),SS->getBallSpeedMax());
}

//21. 传给更靠近对方球门的队友中距离自己最近的（旧21题）
//fiftyExp简版（直接找离对方球门最近的队友）：
soc = leadingPass(WM->getClosestInSetTo(OBJECT_SET_TEAMMATES,WM->getPosOpponentGoal()),1.0);
//严格按题意版（先筛出比我更靠近对方球门的队友，再找离我最近的）：
ObjectT best = OBJECT_ILLEGAL;
double bestDis = 1e9;
double myToGoal = posAgent.getDistanceTo(VecPosition(52.5,0));
int iIndex;
for(ObjectT o = WM->iterateObjectStart(iIndex,OBJECT_SET_TEAMMATES);
    o != OBJECT_ILLEGAL;
    o = WM->iterateObjectNext(iIndex,OBJECT_SET_TEAMMATES)){
    VecPosition p = WM->getGlobalPosition(o);
    if(p.getDistanceTo(VecPosition(52.5,0)) < myToGoal){   //比我更靠近球门
        double d = p.getDistanceTo(posAgent);
        if(d < bestDis){ bestDis = d; best = o; }
    }
}
WM->iterateObjectDone(iIndex);
if(best != OBJECT_ILLEGAL)
    soc = leadingPass(best,1.0);
else
    soc = dribble((VecPosition(52.5,0)-posAgent).getDirection(),DRIBBLE_WITHBALL);

//22. 前方[-30°,30°]、20米内搜队友；有则传球，否则自己带球（旧22题）
ObjectT mate = OBJECT_ILLEGAL;
int iIndex;
for(ObjectT o = WM->iterateObjectStart(iIndex,OBJECT_SET_TEAMMATES);
    o != OBJECT_ILLEGAL;
    o = WM->iterateObjectNext(iIndex,OBJECT_SET_TEAMMATES)){
    VecPosition p = WM->getGlobalPosition(o);
    double dis = p.getDistanceTo(posAgent);
    AngDeg a = (p-posAgent).getDirection();
    if(dis <= 20 && a >= -30 && a <= 30){ mate = o; break; }
}
WM->iterateObjectDone(iIndex);
if(mate != OBJECT_ILLEGAL)
    soc = leadingPass(mate,1.0);
else
    soc = dribble(0.0,DRIBBLE_WITHBALL);

//23. 统计y>0与y<0两侧对方球员数，传给对手少的一侧x最大的队友（旧26题）
//在WorldModel.h的public:后加：
//    bool     isOwnSideOpponentMost();
//    ObjectT  getMaxXTeammateInSide(bool isOwnSide);
//在WorldModel.cpp中加：
ObjectT WorldModel::getMaxXTeammateInSide(bool isOwnSide)
{
    int iIndex;
    ObjectT maxMate = OBJECT_ILLEGAL;
    for(ObjectT o = iterateObjectStart(iIndex,OBJECT_SET_TEAMMATES);
        o != OBJECT_ILLEGAL;
        o = iterateObjectNext(iIndex,OBJECT_SET_TEAMMATES)){
        VecPosition oPos = getGlobalPosition(o);
        if((isOwnSide && oPos.getY() >= 0) || (!isOwnSide && oPos.getY() <= 0)){
            if(maxMate == OBJECT_ILLEGAL || oPos.getX() > getGlobalPosition(maxMate).getX())
                maxMate = o;
        }
    }
    iterateObjectDone(iIndex);
    return maxMate;
}
bool WorldModel::isOwnSideOpponentMost()   //true表示y>0一侧对手更多
{
    int ownSideCount = 0, count = 0;
    int iIndex;
    for(ObjectT o = iterateObjectStart(iIndex,OBJECT_SET_OPPONENTS);
        o != OBJECT_ILLEGAL;
        o = iterateObjectNext(iIndex,OBJECT_SET_OPPONENTS)){
        if(getGlobalPosition(o).getY() > 0) ownSideCount++;
        count++;
    }
    iterateObjectDone(iIndex);
    return ownSideCount > count - ownSideCount;
}
//deMeer5()拿球分支中：
{
    ObjectT mate = WM->getMaxXTeammateInSide(!WM->isOwnSideOpponentMost());     //注意取反
    if(mate != OBJECT_ILLEGAL)
        soc = leadingPass(mate,1.0);
    else
        soc = dribble((VecPosition(52.5,0)-posAgent).getDirection(),DRIBBLE_WITHBALL);
}

//24. 7米内>=2名对方逼抢，则传给己方x坐标最大的队友（旧27题）
//WorldModel.h的public:后加： ObjectT getMaxXTeammate();
//WorldModel.cpp中加：
ObjectT WorldModel::getMaxXTeammate()
{
    int iIndex;
    ObjectT maxMate = OBJECT_ILLEGAL;
    for(ObjectT o = iterateObjectStart(iIndex,OBJECT_SET_TEAMMATES);
        o != OBJECT_ILLEGAL;
        o = iterateObjectNext(iIndex,OBJECT_SET_TEAMMATES)){
        VecPosition oPos = getGlobalPosition(o);
        if(maxMate == OBJECT_ILLEGAL || oPos.getX() > getGlobalPosition(maxMate).getX())
            maxMate = o;
    }
    iterateObjectDone(iIndex);
    return maxMate;
}
//拿球分支：
Circle cir(posAgent,7);
int num = WM->getNrInSetInCircle(OBJECT_SET_OPPONENTS,cir);
if(num >= 2){   //【修正】题意是>=2，fiftyExp原文写num>2
    ObjectT mate = WM->getMaxXTeammate();
    soc = leadingPass(mate,1.0);
}
else{
    soc = dribble((VecPosition(52.5,0)-posAgent).getDirection(),DRIBBLE_WITHBALL);
}

//25. 7米内没有对方球员，则朝(52.5,0)快速带球（旧30题）
Circle cir(posAgent,7);
int num = WM->getNrInSetInCircle(OBJECT_SET_OPPONENTS,cir);
if(num == 0)
    soc = dribble((VecPosition(52.5,0)-posAgent).getDirection(),DRIBBLE_FAST);
else
    soc = dribble((VecPosition(52.5,0)-posAgent).getDirection(),DRIBBLE_WITHBALL);

//26. 前方无己方队友且5米内有2名及以上对方球员，传给最近队友（旧32题）
//在WorldModel里参照isOpponentAtAngle()写isTeammateAtAngle()
//（把函数里的OBJECT_SET_OPPONENTS换成OBJECT_SET_TEAMMATES即可）
//注意参数语义同上：isTeammateAtAngle(方向角, 距离)，窗口±60°写死
Circle cir(posAgent,5);   //【修正】题意是5米，不是7米
int num = WM->getNrInSetInCircle(OBJECT_SET_OPPONENTS,cir);
if(WM->isTeammateAtAngle(0.0,30.0) == false && num >= 2){   //正前方30米内无队友
    soc = leadingPass(WM->getClosestInSetTo(OBJECT_SET_TEAMMATES,posAgent),1.0);
}

//27. 前方无对方球员则快速带球；距守门员5-10米时向空隙大的一侧射门
if(WM->isOpponentAtAngleEx(-30,30,30) == false){   //前方±30°、30米内无对手
    soc = dribble((VecPosition(52.5,0)-posAgent).getDirection(),DRIBBLE_FAST);
}
VecPosition posGoalie = WM->getGlobalPosition(WM->getOppGoalieType());
double disGoalie = posAgent.getDistanceTo(posGoalie);
if(disGoalie >= 5 && disGoalie <= 10){   //【补全】距离守门员5-10米
    AngDeg ang_goalie = (posGoalie-posAgent).getDirection();
    AngDeg angup   = (VecPosition(52.5, 6.0)-posAgent).getDirection();
    AngDeg angdown = (VecPosition(52.5,-6.0)-posAgent).getDirection();
    if(std::fabs(angup-ang_goalie) > std::fabs(angdown-ang_goalie))
        soc = kickTo(VecPosition(52.5, 6.0),SS->getBallSpeedMax());
    else
        soc = kickTo(VecPosition(52.5,-6.0),SS->getBallSpeedMax());
}

//28. 身处对方禁区直接射门；否则7号带球，其余传给7号
if(WM->isInTheirPenaltyArea(WM->getBallPos())){
    soc = kickTo(VecPosition(52.5,0),SS->getBallSpeedMax());
}
else{
    if(WM->getPlayerNumber() == 7)
        soc = dribble((VecPosition(52.5,0)-posAgent).getDirection(),DRIBBLE_FAST);
    else
        soc = leadingPass(OBJECT_TEAMMATE_7,1.0);
}

//29. 8、9、10、11号可踢球时：前方无对方球员则快速带球；距守门员5-10米时向空隙大的一侧射门
int myNum = WM->getPlayerNumber();
if(myNum==8 || myNum==9 || myNum==10 || myNum==11){
    VecPosition posGoalie = WM->getGlobalPosition(WM->getOppGoalieType());
    double disGoalie = posAgent.getDistanceTo(posGoalie);
    if(disGoalie >= 5 && disGoalie <= 10){
        AngDeg ang_goalie = (posGoalie-posAgent).getDirection();
        AngDeg angup   = (VecPosition(52.5, 6.0)-posAgent).getDirection();
        AngDeg angdown = (VecPosition(52.5,-6.0)-posAgent).getDirection();
        if(std::fabs(angup-ang_goalie) > std::fabs(angdown-ang_goalie))
            soc = kickTo(VecPosition(52.5, 6.0),SS->getBallSpeedMax());
        else
            soc = kickTo(VecPosition(52.5,-6.0),SS->getBallSpeedMax());
    }
    else if(WM->isOpponentAtAngleEx(-30,30,30) == false){   //前方±30°、30米内无对手
        soc = dribble((VecPosition(52.5,0)-posAgent).getDirection(),DRIBBLE_FAST);
    }
}
else{
    soc = leadingPass(OBJECT_TEAMMATE_10,1.0);   //其余球员交给前锋
}

//30. 距球门>25米快速带球推进；<=25米优先传给5米内队友，没有则慢速带球
double disGoal = posAgent.getDistanceTo(VecPosition(52.5,0));
if(disGoal > 25){
    soc = dribble((VecPosition(52.5,0)-posAgent).getDirection(),DRIBBLE_FAST);
}
else{
    Circle cir(posAgent,5);
    if(WM->getNrInSetInCircle(OBJECT_SET_TEAMMATES,cir) > 0)
        soc = leadingPass(WM->getClosestInSetTo(OBJECT_SET_TEAMMATES,posAgent),1.0);
    else
        soc = dribble((VecPosition(52.5,0)-posAgent).getDirection(),DRIBBLE_SLOW);
}

//31. 身后（x负方向一侧）7米内有对方逼抢，向前斜30度踢出约8米摆脱后继续带球
Circle cir(posAgent,7);
bool backPress = false;
int iIndex;
for(ObjectT o = WM->iterateObjectStart(iIndex,OBJECT_SET_OPPONENTS);
    o != OBJECT_ILLEGAL;
    o = WM->iterateObjectNext(iIndex,OBJECT_SET_OPPONENTS)){
    VecPosition p = WM->getGlobalPosition(o);
    if(p.getDistanceTo(posAgent) <= 7 && p.getX() < posAgent.getX()){ backPress = true; break; }
}
WM->iterateObjectDone(iIndex);
if(backPress){
    //VecPosition极坐标构造：(模长, 全局角度, POLAR)
    VecPosition target = posAgent + VecPosition(8.0, WM->getAgentGlobalBodyAngle()+30, POLAR);
    soc = kickTo(target,2.0);
}
else{
    soc = dribble((VecPosition(52.5,0)-posAgent).getDirection(),DRIBBLE_WITHBALL);
}

//32. 在对方禁区内且周围6米无对方球员，随机选一侧球门角射门；否则传最近队友
Circle cir(posAgent,6);
if(WM->isInTheirPenaltyArea(WM->getBallPos()) &&
   WM->getNrInSetInCircle(OBJECT_SET_OPPONENTS,cir) == 0){
    //用周期数当"随机"：偶数上角、奇数下角
    double y = (WM->getCurrentCycle()%2 == 0) ? 6.0 : -6.0;
    soc = kickTo(VecPosition(52.5,y),SS->getBallSpeedMax());
}
else{
    soc = leadingPass(WM->getClosestInSetTo(OBJECT_SET_TEAMMATES,posAgent),1.0);
}

---

// 二、playOn 模式下 · 无球跑位（180行策略）

//33. 10号控球向前带球；5号无球跟随，与10号同一水平线、距离保持5米（旧17题）
//--- 134行拿球策略 ---
if(WM->getPlayerNumber() != 10)
    soc = leadingPass(OBJECT_TEAMMATE_10,1.0);
else
    soc = dribble(0.0,DRIBBLE_SLOW);
//--- 180行跑位策略 ---
if(WM->getPlayerNumber() == 5){
    VecPosition pos(WM->getBallPos().getX()-5, WM->getBallPos().getY());
    soc = moveToPos(pos,20);
}

//34. 计算越位参考线；有队友可接球且自己是10号，跑向(越位线x-2, 自身y)（旧28题）
if(WM->isBallInOurPossesion() && WM->getPlayerNumber() == 10){
    soc = moveToPos(VecPosition(WM->getOffsideX()-2, posAgent.getY()),20);
}

//35. 构建球运动轨迹直线；自身到直线距离<4米则跑向垂足点（旧29题）
//在 formations->setFormation(...) 之后加入：
Line ballRun = Line::makeLineFromPositionAndAngle(WM->getBallPos(), WM->getBallDirection());
if(ballRun.getDistanceWithPoint(WM->getAgentGlobalPosition()) < 4){
    soc = moveToPos(ballRun.getPointOnLineClosestTo(WM->getAgentGlobalPosition()),20);
}

//36. 距球最近的是己方队友、自己是10号且x<30，跑向(ball_x, ball_y+10)（旧31题）
if(WM->isBallInOurPossesion() && WM->getPlayerNumber() == 10 && posAgent.getX() < 30){
    VecPosition pos = WM->getBallPos() + VecPosition(0,10);
    soc = moveToPos(pos,20);   //【修正】fiftyExp原文漏了soc=和大括号
}

//37. 无球状态：持球者是己方7号，跑到7号侧前方6米接应
if(WM->getClosestInSetTo(OBJECT_SET_TEAMMATES,WM->getBallPos()) == OBJECT_TEAMMATE_7
   && WM->getPlayerNumber() != 7){
    VecPosition pos7 = WM->getGlobalPosition(OBJECT_TEAMMATE_7);
    //侧前方：朝对方球门方向偏45度、距离6米的点
    AngDeg angGoal = (VecPosition(52.5,0)-pos7).getDirection();
    VecPosition pos = pos7 + VecPosition(6.0, angGoal+45, POLAR);
    soc = moveToPos(pos,20);
}

//38. 可踢球时：把球传给距对方球门第二近的队友；无队友可用则自己带球
ObjectT first = OBJECT_ILLEGAL, second = OBJECT_ILLEGAL;
double d1 = 1e9, d2 = 1e9;
int iIndex;
for(ObjectT o = WM->iterateObjectStart(iIndex,OBJECT_SET_TEAMMATES);
    o != OBJECT_ILLEGAL;
    o = WM->iterateObjectNext(iIndex,OBJECT_SET_TEAMMATES)){
    double d = WM->getGlobalPosition(o).getDistanceTo(VecPosition(52.5,0));
    if(d < d1){ d2 = d1; second = first; d1 = d; first = o; }
    else if(d < d2){ d2 = d; second = o; }
}
WM->iterateObjectDone(iIndex);
if(second != OBJECT_ILLEGAL)
    soc = leadingPass(second,1.0);
else
    soc = dribble((VecPosition(52.5,0)-posAgent).getDirection(),DRIBBLE_WITHBALL);

---

// 三、playOn 模式下 · 防守行为（180行策略）

//39. 防守状态，5号盯防对方拿球球员，距离保持5米（旧18题）
if(!WM->isBallInOurPossesion() && WM->getPlayerNumber() == 5){   //球不在我方脚下=防守
    //拿球的就是对方球员，站在球后方5米处即与持球者保持5米
    VecPosition pos(WM->getBallPos().getX()-5, WM->getBallPos().getY());
    soc = moveToPos(pos,20);
}

//40. 防守状态，2号、4号同时盯防对方拿球球员（旧19题）
if(WM->getAgentObjectType() == OBJECT_TEAMMATE_2){
    VecPosition pos(WM->getBallPos().getX()-5, WM->getBallPos().getY());
    soc = moveToPos(pos,20);   //【修正】fiftyExp原文漏分号
}
if(WM->getAgentObjectType() == OBJECT_TEAMMATE_4){
    VecPosition pos(WM->getBallPos().getX(), WM->getBallPos().getY()-5);
    soc = moveToPos(pos,20);
}

//41. 对方10号拿球时，2、3、4号跑位盯防对方10号（旧41题）
int num = WM->getPlayerNumber();
if(WM->getClosestInSetTo(OBJECT_SET_OPPONENTS,WM->getBallPos()) == OBJECT_OPPONENT_10
   && (num==2 || num==3 || num==4)){
    VecPosition pos = WM->getMarkingPosition(WM->getGlobalPosition(OBJECT_OPPONENT_10),2.0,MARK_BALL);
    soc = moveToPos(pos,PS->getPlayerWhenToTurnAngle());
    ACT->putCommandInQueue(soc);
}

//42. 对方9号拿球：2、3、4号中距球最近者盯防9号，其余盯防距自己最近的对方球员（旧42题）
if(WM->getClosestInSetTo(OBJECT_SET_OPPONENTS,WM->getBallPos()) == OBJECT_OPPONENT_9){
    ObjectT closestObject = OBJECT_TEAMMATE_2;
    VecPosition oppPos9 = WM->getGlobalPosition(OBJECT_OPPONENT_9);
    double minDis = WM->getGlobalPosition(OBJECT_TEAMMATE_2).getDistanceTo(oppPos9);
    double dis = WM->getGlobalPosition(OBJECT_TEAMMATE_3).getDistanceTo(oppPos9);
    if(dis < minDis){ minDis = dis; closestObject = OBJECT_TEAMMATE_3; }
    dis = WM->getGlobalPosition(OBJECT_TEAMMATE_4).getDistanceTo(oppPos9);
    if(dis < minDis){ minDis = dis; closestObject = OBJECT_TEAMMATE_4; }

    if(WM->getAgentObjectType() == closestObject){
        VecPosition pos = WM->getMarkingPosition(WM->getGlobalPosition(OBJECT_OPPONENT_9),2.0,MARK_BALL);
        soc = moveToPos(pos,PS->getPlayerWhenToTurnAngle());
    }
    else{
        ObjectT opp = WM->getClosestInSetTo(OBJECT_SET_OPPONENTS,WM->getAgentObjectType());
        VecPosition oppPos = WM->getMarkingPosition(WM->getGlobalPosition(opp),2.0,MARK_BALL);
        soc = moveToPos(oppPos,40);
    }
}

//43. 对方会比我方更早触球：距球最近的我方球员盯球，其余盯防最近的对方球员（旧43题）
//WorldModel.h加： bool func43();
//WorldModel.cpp加：
bool WorldModel::func43()
{
    ObjectT opp = getFastestInSetTo(OBJECT_SET_OPPONENTS,OBJECT_BALL);
    double oppDisToBall = getGlobalPosition(opp).getDistanceTo(getBallPos());
    ObjectT teammate = getFastestInSetTo(OBJECT_SET_TEAMMATES,OBJECT_BALL);
    double teammateDisToBall = getGlobalPosition(teammate).getDistanceTo(getBallPos());
    return oppDisToBall < teammateDisToBall;
}
//180行：
if(WM->func43()){
    ObjectT closest = WM->getClosestInSetTo(OBJECT_SET_TEAMMATES,WM->getBallPos());
    if(WM->getAgentObjectType() == closest){
        soc = moveToPos(WM->getBallPos(),40);   //【修正】fiftyExp原文错用OPPONENTS查找
    }
    else{
        ObjectT opp = WM->getClosestInSetTo(OBJECT_SET_OPPONENTS,WM->getAgentObjectType());
        VecPosition oppPos = WM->getMarkingPosition(WM->getGlobalPosition(opp),2.0,MARK_BALL);
        soc = moveToPos(oppPos,PS->getPlayerWhenToTurnAngle());
    }
}

//44. 对方11号拿球：7号从左侧、8号从右侧跑位断球（旧44题）
if(WM->getClosestInSetTo(OBJECT_SET_OPPONENTS,WM->getBallPos()) == OBJECT_OPPONENT_11){
    VecPosition oppPos = WM->getGlobalPosition(OBJECT_OPPONENT_11);
    if(WM->getAgentObjectType() == OBJECT_TEAMMATE_7){   //【修正】原文==7应为OBJECT_TEAMMATE_7
        VecPosition selfPos = WM->getGlobalPosition(OBJECT_TEAMMATE_7);
        if(selfPos.getDistanceTo(oppPos) > 5)
            soc = moveToPos(VecPosition(oppPos.getX(),oppPos.getY()+4),40);  //左侧
        else
            soc = intercept(false);
    }
    else if(WM->getAgentObjectType() == OBJECT_TEAMMATE_8){
        VecPosition selfPos = WM->getGlobalPosition(OBJECT_TEAMMATE_8);
        if(selfPos.getDistanceTo(oppPos) > 5)
            soc = moveToPos(VecPosition(oppPos.getX(),oppPos.getY()-4),40);  //右侧
        else
            soc = intercept(false);
    }
}

//45. 防守状态：6号跟对方9号、7号跟对方10号、8号跟对方11号（旧45题）
if(!WM->isBallInOurPossesion()){
    if(WM->getAgentObjectType() == OBJECT_TEAMMATE_6)
        soc = moveToPos(WM->getGlobalPosition(OBJECT_OPPONENT_9),40);
    if(WM->getAgentObjectType() == OBJECT_TEAMMATE_7)
        soc = moveToPos(WM->getGlobalPosition(OBJECT_OPPONENT_10),40);
    if(WM->getAgentObjectType() == OBJECT_TEAMMATE_8)
        soc = moveToPos(WM->getGlobalPosition(OBJECT_OPPONENT_11),40);
    ACT->putCommandInQueue(soc);
    ACT->putCommandInQueue(turnNeckToObject(OBJECT_BALL,soc));
}

//46. 我方距球最近的2人盯防对方距球最近者；其余人球兼顾，卡位最近对方球员接球路线前方
ObjectT oppNear = WM->getClosestInSetTo(OBJECT_SET_OPPONENTS,WM->getBallPos());
ObjectT tm1 = WM->getClosestInSetTo(OBJECT_SET_TEAMMATES,WM->getBallPos());
ObjectT tm2 = WM->getSecondClosestInSetTo(OBJECT_SET_TEAMMATES,WM->getBallPos());
if(WM->getAgentObjectType() == tm1 || WM->getAgentObjectType() == tm2){
    VecPosition pos = WM->getMarkingPosition(WM->getGlobalPosition(oppNear),2.0,MARK_BALL);
    soc = moveToPos(pos,40);
}
else{
    //人球兼顾：站到距自己最近的对方球员的盯防点（球与该球员之间）
    ObjectT opp = WM->getClosestInSetTo(OBJECT_SET_OPPONENTS,WM->getAgentObjectType());
    VecPosition pos = WM->getMarkingPosition(WM->getGlobalPosition(opp),2.0,MARK_BALL);
    soc = moveToPos(pos,20);
}

//47. 我方距球最近者距球<12米则全速逼抢；其余收缩到x>-20区域落位
ObjectT tm1 = WM->getClosestInSetTo(OBJECT_SET_TEAMMATES,WM->getBallPos());
if(WM->getAgentObjectType() == tm1
   && posAgent.getDistanceTo(WM->getBallPos()) < 12){
    soc = intercept(false);   //全速上前逼抢球
}
else{
    if(posAgent.getX() < -20)   //收缩到 x>-20 区域
        soc = moveToPos(VecPosition(-20,posAgent.getY()),20);
}

---

// 四、本方角球 / 本方界外球 · 进攻行为 （代码应放置在PlayerTeams.cpp的if(WM->isBeforeKickOff())和开球决策的else之间，详情可见老师的实验演示视频）

//48. 本方角球：10号跑向角球点，可踢时传给9号；9号跑向角球点附近场内点接应；其余跑本位（旧33题）
//moveToPos(坐标,角度阈值); 角度阈值指目标方向和身体朝向夹角应小于该角，否则先转身再跑
else if(WM->isCornerKickUs())
{
    if(WM->getAgentObjectType() == OBJECT_TEAMMATE_10){
        if(WM->isBallKickable())
            soc = leadingPass(OBJECT_TEAMMATE_9,1.0);
        else
            soc = moveToPos(WM->getBallPos(),PS->getPlayerWhenToTurnAngle());
    }
    if(WM->getAgentObjectType() == OBJECT_TEAMMATE_9){
        VecPosition ball = WM->getBallPos();
        //朝场内方向偏移5米的点（保证在场内）
        VecPosition pos(ball.getX() - ball.getX()/fabs(ball.getX())*5,
                        ball.getY() - ball.getY()/fabs(ball.getY())*5);
        soc = moveToPos(pos,PS->getPlayerWhenToTurnAngle());
    }
    ACT->putCommandInQueue(soc);
    ACT->putCommandInQueue(turnNeckToObject(OBJECT_BALL,soc));
}

//49. 本方界外球：距球最近的队员跑向球；可踢时传给最近的队友（旧34题）
else if(WM->isKickInUs()) {
    int iTmp; double dist;
    if(WM->getFastestInSetTo(OBJECT_SET_TEAMMATES,OBJECT_BALL,&iTmp)
       == WM->getAgentObjectType()){
        if(WM->isBallKickable()){
            ObjectT objTea = WM->getClosestInSetTo(OBJECT_SET_TEAMMATES,WM->getAgentObjectType(),&dist);
            soc = kickTo(WM->getGlobalPosition(objTea),SS->getBallSpeedMax());
        }
        else{
            soc = moveToPos(WM->getBallPos(),20);
        }
        ACT->putCommandInQueue(soc);
    }
}

//50. 本方界外球：球在本方半场左侧由2号发，右侧由5号发；可踢时传给最近队友（旧35题）
else if(WM->isKickInUs()) {
    VecPosition posBall = WM->getBallPos();
    double dist;
    if((posBall.getX()<0 && posBall.getY()<0 && WM->getPlayerNumber()==2) ||
       (posBall.getX()<0 && posBall.getY()>0 && WM->getPlayerNumber()==5)){
        if(WM->isBallKickable()){
            ObjectT objTea = WM->getClosestInSetTo(OBJECT_SET_TEAMMATES,WM->getAgentObjectType(),&dist);
            soc = kickTo(WM->getGlobalPosition(objTea),SS->getBallSpeedMax());
        }
        else{
            soc = moveToPos(WM->getBallPos(),20);
        }
        ACT->putCommandInQueue(soc);
    }
}

//51. 本方界外球：距球第二近的队友向球跑动，停在距球7米处（旧36题）
else if(WM->isKickInUs())
{
    ObjectT o = WM->getSecondClosestInSetTo(OBJECT_SET_TEAMMATES,OBJECT_BALL);
    double dis = posAgent.getDistanceTo(WM->getBallPos());
    if(WM->getAgentObjectType() == o && dis > 7){
        soc = moveToPos(WM->getBallPos(),20);
        ACT->putCommandInQueue(soc);
    }
}

//52. 本方界外球：4号且不是距球最近者，跑向(ball_x+5, ball_y)，不能出界（旧37题+边界判断）
else if(WM->isKickInUs())
{
    if(WM->getPlayerNumber() == 4 &&
       WM->getClosestInSetTo(OBJECT_SET_TEAMMATES,OBJECT_BALL) != OBJECT_TEAMMATE_4){
        VecPosition pos = WM->getBallPos() + VecPosition(5,0);
        //边界判断：不能超出球场
        if(pos.getX() >  PITCH_LENGTH/2.0) pos.setX( PITCH_LENGTH/2.0);
        if(pos.getX() < -PITCH_LENGTH/2.0) pos.setX(-PITCH_LENGTH/2.0);
        if(pos.getY() >  PITCH_WIDTH/2.0)  pos.setY( PITCH_WIDTH/2.0);
        if(pos.getY() < -PITCH_WIDTH/2.0)  pos.setY(-PITCH_WIDTH/2.0);
        soc = moveToPos(pos,20);
        ACT->putCommandInQueue(soc);
        ACT->putCommandInQueue(turnNeckToObject(OBJECT_BALL,soc));
    }
}

//53. 本方界外球：距球第二近的队友，跑向"距球12米内且距对方球门(52.5,0)最近"的场内点（旧38题）
//即在球与球门连线上、距球12米的点
else if(WM->isKickInUs())
{
    ObjectT o = WM->getSecondClosestInSetTo(OBJECT_SET_TEAMMATES,OBJECT_BALL);
    if(WM->getAgentObjectType() == o){
        VecPosition ball = WM->getBallPos();
        VecPosition goal(52.5,0);
        AngDeg ang = (goal-ball).getDirection();
        VecPosition pos = ball + VecPosition(12.0,ang,POLAR);   //球指向球门方向12米处
        if(pos.getX() >  PITCH_LENGTH/2.0) pos.setX( PITCH_LENGTH/2.0);
        if(pos.getY() >  PITCH_WIDTH/2.0)  pos.setY( PITCH_WIDTH/2.0);
        if(pos.getY() < -PITCH_WIDTH/2.0)  pos.setY(-PITCH_WIDTH/2.0);
        soc = moveToPos(pos,20);
    }
    ACT->putCommandInQueue(soc);
}

//54. 本方界外球：最近者跑向球发球；我既不是发球人也不是第二近，则朝球方向跑动接应（旧39题）
else if(WM->isKickInUs())
{
    ObjectT o1 = WM->getClosestInSetTo(OBJECT_SET_TEAMMATES,OBJECT_BALL);
    ObjectT o2 = WM->getSecondClosestInSetTo(OBJECT_SET_TEAMMATES,OBJECT_BALL);
    if(WM->getAgentObjectType() == o1){
        if(WM->isBallKickable())
            soc = leadingPass(WM->getClosestInSetTo(OBJECT_SET_TEAMMATES,posAgent),1.0);
        else
            soc = moveToPos(WM->getBallPos(),20);
    }
    else if(WM->getAgentObjectType() != o2){
        //朝球的方向跑动接应（跑到距球约10米的接应点）
        AngDeg ang = (WM->getBallPos()-posAgent).getDirection();
        VecPosition pos = WM->getBallPos() + VecPosition(10.0,ang+180,POLAR);
        soc = moveToPos(pos,20);
    }
    ACT->putCommandInQueue(soc);
}

//55. 本方界外球：最近者发球；我不是第二近，则跑向(第二近x+10, 第二近y+5)，出界则跑最近边界点（旧40题）
else if(WM->isKickInUs())
{
    ObjectT o1 = WM->getClosestInSetTo(OBJECT_SET_TEAMMATES,OBJECT_BALL);
    ObjectT o2 = WM->getSecondClosestInSetTo(OBJECT_SET_TEAMMATES,OBJECT_BALL);
    VecPosition o2Pos = WM->getGlobalPosition(o2);
    if(WM->getAgentObjectType() == o1){
        if(WM->isBallKickable())
            soc = leadingPass(WM->getClosestInSetTo(OBJECT_SET_TEAMMATES,posAgent),1.0);
        else
            soc = moveToPos(WM->getBallPos(),20);
    }
    else if(WM->getAgentObjectType() != o2){
        VecPosition target(o2Pos.getX()+10, o2Pos.getY()+5);
        //超出边界则钳到最近的场内边界点
        if(target.getX() >  PITCH_LENGTH/2.0) target.setX( PITCH_LENGTH/2.0);
        if(target.getX() < -PITCH_LENGTH/2.0) target.setX(-PITCH_LENGTH/2.0);
        if(target.getY() >  PITCH_WIDTH/2.0)  target.setY( PITCH_WIDTH/2.0);
        if(target.getY() < -PITCH_WIDTH/2.0)  target.setY(-PITCH_WIDTH/2.0);
        soc = moveToPos(target,20);
    }
    ACT->putCommandInQueue(soc);
}

---

// 五、对方角球 / 对方界外球 · 防守行为

//56. 对方界外球：4号跑向距球最近的对方球员盯防（旧46题）
else if(WM->isKickInThem())
{
    if(WM->getPlayerNumber() == 4){
        ObjectT opp = WM->getClosestInSetTo(OBJECT_SET_OPPONENTS,OBJECT_BALL);
        //【修正】fiftyExp原文 getCloesestInsetTo 拼写错误
        VecPosition pos = WM->getMarkingPosition(WM->getGlobalPosition(opp),2.0,MARK_BALL);
        soc = moveToPos(pos,PS->getPlayerWhenToTurnAngle());
    }
    ACT->putCommandInQueue(soc);
}

//57. 对方界外球：我是距球第二近的己方队员，则盯防距球第二近的对方球员（旧47题）
else if(WM->isKickInThem())
{
    if(WM->getSecondClosestInSetTo(OBJECT_SET_TEAMMATES,OBJECT_BALL)
       == WM->getAgentObjectType()){
        ObjectT opp = WM->getSecondClosestInSetTo(OBJECT_SET_OPPONENTS,OBJECT_BALL);
        //【修正】fiftyExp原文错写成OBJECT_SET_TEAMMATES
        soc = moveToPos(WM->getGlobalPosition(opp),PS->getPlayerWhenToTurnAngle());
    }
    ACT->putCommandInQueue(soc);
}

//58. 对方角球：球在我方左侧，2、3、6、7号均布在球为圆心10米场内圆弧上；
//             球在我方右侧，4、5、7、8号均布
else if(WM->isCornerKickThem())
{
    VecPosition ball = WM->getBallPos();
    int num = WM->getPlayerNumber();
    double startAng = 0, step = 30;   //起始角与间隔可自定义
    int idx = -1;
    if(ball.getY() < 0){   //我方场地左侧
        if(num==2) idx=0; else if(num==3) idx=1;
        else if(num==6) idx=2; else if(num==7) idx=3;
    }
    else{                  //我方场地右侧
        if(num==4) idx=0; else if(num==5) idx=1;
        else if(num==7) idx=2; else if(num==8) idx=3;
    }
    if(idx >= 0){
        VecPosition pos = ball + VecPosition(10.0, startAng+idx*step, POLAR);
        //保证在场内
        if(pos.getX() >  PITCH_LENGTH/2.0) pos.setX( PITCH_LENGTH/2.0);
        if(pos.getX() < -PITCH_LENGTH/2.0) pos.setX(-PITCH_LENGTH/2.0);
        if(pos.getY() >  PITCH_WIDTH/2.0)  pos.setY( PITCH_WIDTH/2.0);
        if(pos.getY() < -PITCH_WIDTH/2.0)  pos.setY(-PITCH_WIDTH/2.0);
        soc = moveToPos(pos,20);
    }
    ACT->putCommandInQueue(soc);
}

//59. 对方界外球：我不是距球最近的队友且x>0，跑向(self_x-10, self_y)，不可出界（旧48题）
else if(WM->isKickInThem())
{
    if(WM->getClosestInSetTo(OBJECT_SET_TEAMMATES,OBJECT_BALL) != WM->getAgentObjectType()
       && posAgent.getX() > 0){
        VecPosition pos = posAgent + VecPosition(-10,0);
        if(pos.getX() < -PITCH_LENGTH/2.0) pos.setX(-PITCH_LENGTH/2.0);  //边界校验
        soc = moveToPos(pos,20);
    }
    ACT->putCommandInQueue(soc);
}

//60. 对方界外球：5米内有对方球员，跑到其附近且x比对方大2，保证在场内（旧49题）
else if(WM->isKickInThem())
{
    Circle cir(posAgent,5);
    if(WM->getNrInSetInCircle(OBJECT_SET_OPPONENTS,cir) != 0){
        ObjectT o = WM->getClosestInSetTo(OBJECT_SET_OPPONENTS,posAgent);
        VecPosition poso = WM->getGlobalPosition(o);
        VecPosition pos(poso.getX()+2, poso.getY());
        if(pos.getX() > PITCH_LENGTH/2.0) pos.setX(PITCH_LENGTH/2.0);   //场内校验
        soc = moveToPos(pos,20);
    }
    ACT->putCommandInQueue(soc);
}

//61. 对方界外球：球x<0（本方半场），4号或5号盯防距球第二近的对方球员（旧50题）
else if(WM->isKickInThem())
{
    if(WM->getBallPos().getX() < 0 &&
       (WM->getPlayerNumber()==4 || WM->getPlayerNumber()==5)){   //【修正】原文getPlayerNumber缺括号
        ObjectT o = WM->getSecondClosestInSetTo(OBJECT_SET_OPPONENTS,OBJECT_BALL);
        soc = moveToPos(WM->getGlobalPosition(o),20);
        ACT->putCommandInQueue(soc);
    }
}

//62. 对方角球：球在我方左侧，2、3号盯防距球第二近的对方球员；右侧则4、5号
else if(WM->isCornerKickThem())
{
    VecPosition ball = WM->getBallPos();
    int num = WM->getPlayerNumber();
    bool mark = (ball.getY()<0 && (num==2||num==3)) ||
                (ball.getY()>0 && (num==4||num==5));
    if(mark){
        ObjectT o = WM->getSecondClosestInSetTo(OBJECT_SET_OPPONENTS,OBJECT_BALL);
        soc = moveToPos(WM->getGlobalPosition(o),20);
    }
    ACT->putCommandInQueue(soc);
}
