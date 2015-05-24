
'
' branching 1
'
Sub Br1
  Dim a As Integer
  Dim b As Integer
  If a <> b Then
    a = 7
  End If
End Sub

'
' branching 2
'
Sub Br2
  Dim a As Integer
  Dim b As Integer
  Dim c As Integer
  If a <> b Then
    a = 7
  Else
    b = 5
    c = a * 2
  End If
End Sub

'
' branching 3
'
Sub Br3
  Dim a As Integer
  Dim b As Integer
  Dim c As Integer
  If a <> b Then
    a = 7
  Else
    b = 77
    If a > b Then 
        b = 5
    Else
    	c = a * 2
    End If
  End If
  Print 1
End Sub

Sub Main
End Sub

