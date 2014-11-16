
'
' callee
'
Function f0(x As Integer, y As Integer) As Boolean
  Return True
End Function

'
' caller
'
Function g0() As Boolean
  Dim v As Boolean
  Let v = f0(777, 888)
  Dim pi As Double
  Let pi = 3.14
  Return False
End Function


'
'
'
Function Main() As Integer
  Dim pi As Double
  Let pi = 3.14
  Return 0
End Function

