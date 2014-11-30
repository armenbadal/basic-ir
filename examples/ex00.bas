
Declare Sub S0
Declare Sub S1 x As Double, y As Integer, z As Boolean

Function Main() As Integer
  S0
  S1 3.14, 888, True
  Return 0
End Function

Sub S0
  Print 777
End Sub

Sub S1 x As Double, y As Integer, z As Boolean
  Print x
  Print y
  Print z
End Sub


