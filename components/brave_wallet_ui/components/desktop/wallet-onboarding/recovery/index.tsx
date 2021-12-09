import * as React from 'react'

import {
  StyledWrapper,
  Title,
  Description,
  TermsRow,
  CopyButton,
  WarningBox,
  WarningText,
  DisclaimerText,
  DisclaimerColumn,
  AlertIcon,
  RecoveryBubble,
  RecoveryBubbleText,
  RecoveryPhraseContainer,
  BigCheckMark
} from './style'
import { Tooltip } from '../../../shared'
import { NavButton } from '../../../extension'
import { getLocale } from '../../../../../common/locale'
import { Checkbox } from 'brave-ui'

export interface Props {
  onSubmit: () => void
  isRecoveryTermsAccepted: boolean
  onSubmitTerms: (key: string, selected: boolean) => void
  recoverPhrase: string[]
  recoverPhraseCopied: boolean
  onCopy: () => void
}

function OnboardingBackup (props: Props) {
  const { onSubmit, isRecoveryTermsAccepted, onSubmitTerms, recoverPhrase, recoverPhraseCopied, onCopy } = props

  return (
    <StyledWrapper>
      <Title>{getLocale('braveWalletRecoveryTitle')}</Title>
      <Description>{getLocale('braveWalletRecoveryDescription')}</Description>
      <WarningBox>
        <AlertIcon />
        <DisclaimerColumn>
          <DisclaimerText><WarningText>{getLocale('braveWalletRecoveryWarning1')} </WarningText>{getLocale('braveWalletRecoveryWarning2')}</DisclaimerText>
          <DisclaimerText>{getLocale('braveWalletRecoveryWarning3')}</DisclaimerText>
        </DisclaimerColumn>
      </WarningBox>
      <RecoveryPhraseContainer>
        {recoverPhrase.map((word, index) =>
          <RecoveryBubble key={index}>
            <RecoveryBubbleText>{index + 1}. {word}</RecoveryBubbleText>
          </RecoveryBubble>
        )}
      </RecoveryPhraseContainer>
      <Tooltip text={getLocale('braveWalletToolTipCopyToClipboard')}>
        <CopyButton onClick={onCopy}>
          {recoverPhraseCopied && <BigCheckMark />}
          {recoverPhraseCopied ? 'Copied' : getLocale('braveWalletButtonCopy')}
        </CopyButton>
      </Tooltip>
      <TermsRow>
        <Checkbox value={{ backedUp: isRecoveryTermsAccepted }} onChange={onSubmitTerms}>
          <div data-key='backedUp'>{getLocale('braveWalletRecoveryTerms')}</div>
        </Checkbox>
      </TermsRow>
      <NavButton disabled={!isRecoveryTermsAccepted} buttonType='primary' text={getLocale('braveWalletButtonContinue')} onSubmit={onSubmit} />
    </StyledWrapper>
  )
}

export default OnboardingBackup
