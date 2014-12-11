
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
  v = f0(777, 888)
  Dim pi As Double
  pi = 3.14
  Return False
End Function


'
'
'
Sub Main
  Dim pi As Double
  pi = 3.14
End Sub

