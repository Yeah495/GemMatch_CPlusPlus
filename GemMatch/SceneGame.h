#pragma once
/*  a. 技术建议：使用 QGraphicsView 和 QGraphicsScene 框架。更适合做游戏，能支持平滑的宝石交换动画。
  b. 动画逻辑：当 Model 层发生交换时，View 层负责播放 0.3 秒的移动动画，动画结束再更新数据。*/